#include "VertexBuffer.h"
#include "Engine.h"
#include "GL/GLHelper.h"
#include "RHI/BufferDescriptor.h"

namespace RHI
{

	
VertexBuffer::Builder& VertexBuffer::Builder::vertexCount(uint32_t const vertexCount) noexcept {
    m_Desc.mVertexCount = vertexCount;
    return *this;
}

VertexBuffer::Builder& VertexBuffer::Builder::enableBufferObjects(bool const enabled) noexcept {
    m_Desc.mBufferObjectsEnabled = enabled;
    return *this;
}

VertexBuffer::Builder& VertexBuffer::Builder::bufferCount(uint8_t const bufferCount) noexcept {
    m_Desc.mBufferCount = bufferCount;
    return *this;
}

VertexBuffer::Builder& VertexBuffer::Builder::attribute(VertexAttribute const attribute,
        uint8_t const bufferIndex,
        ElementType const attributeType,
        uint32_t const byteOffset,
        uint8_t byteStride) noexcept {

    size_t const attributeSize = RHI_Internal::GetGLElementSize(attributeType);
    if (byteStride == 0) {
        byteStride = (uint8_t)attributeSize;
    }

    if (size_t(attribute) < MAX_VERTEX_ATTRIBUTE_COUNT &&
        size_t(bufferIndex) < MAX_VERTEX_ATTRIBUTE_COUNT) {

#ifndef NDEBUG
        if (byteOffset & 0x3u) {
            std::cout << "[performance] VertexBuffer::Builder::attribute() "
                             "byteOffset not multiple of 4" << std::endl;
        }
        if (byteStride & 0x3u) {
            std::cout << "[performance] VertexBuffer::Builder::attribute() "
                             "byteStride not multiple of 4" << std::endl;
        }
#endif

        auto& entry = m_Desc.mAttributes[attribute];
        entry.buffer = bufferIndex;
        entry.offset = byteOffset;
        entry.stride = byteStride;
        entry.type = attributeType;
        if (attribute == BONE_INDICES) {
            // BONE_INDICES must always be an integer type
            entry.flags |= Attribute::FLAG_INTEGER_TARGET;
        }

        m_Desc.mDeclaredAttributes.set(attribute);
    } else {
        std::cout << "Ignoring VertexBuffer attribute, the limit of " <<
                MAX_VERTEX_ATTRIBUTE_COUNT << " attributes has been exceeded" << std::endl;
    }
    return *this;
}

VertexBuffer::Builder& VertexBuffer::Builder::normalized(VertexAttribute const attribute,
        bool const normalized) noexcept {
    if (size_t(attribute) < MAX_VERTEX_ATTRIBUTE_COUNT) {
        auto& entry = m_Desc.mAttributes[attribute];
        if (normalized) {
            entry.flags |= Attribute::FLAG_NORMALIZED;
        } else {
            entry.flags &= ~Attribute::FLAG_NORMALIZED;
        }
    }
    return *this;
}


VertexBuffer::Builder& VertexBuffer::Builder::name(const std::string& name) noexcept {
	m_Desc.mName = name;
	return *this;
}

Ref<VertexBuffer> VertexBuffer::Builder::build(RHIDriver& driver) {
    ASSERT(m_Desc.mVertexCount > 0);
    ASSERT(m_Desc.mBufferCount > 0);
    ASSERT(m_Desc.mBufferCount <= MAX_VERTEX_BUFFER_COUNT)

    // Next we check if any unused buffer slots have been allocated. This helps prevent errors
    // because uploading to an unused slot can trigger undefined behavior in the backend.
    auto const& declaredAttributes = m_Desc.mDeclaredAttributes;
    auto const& attributes = m_Desc.mAttributes;
    Util::bitset32 attributedBuffers;

    declaredAttributes.forEachSetBit([&](size_t const j){
        // update set of used buffers
        attributedBuffers.set(attributes[j].buffer);

        // also checks that we don't use an invalid type with integer attributes
        if (attributes[j].flags & Attribute::FLAG_INTEGER_TARGET) {
            using ET = ElementType;
            constexpr uint32_t const invalidIntegerTypes =
                    (1 << (int)ET::FLOAT) |
                    (1 << (int)ET::FLOAT2) |
                    (1 << (int)ET::FLOAT3) |
                    (1 << (int)ET::FLOAT4) |
                    (1 << (int)ET::HALF) |
                    (1 << (int)ET::HALF2) |
                    (1 << (int)ET::HALF3) |
                    (1 << (int)ET::HALF4);
            ASSERT(!(invalidIntegerTypes & (1 << (int)attributes[j].type)))
        }
    });

    ASSERT(attributedBuffers.count() == m_Desc.mBufferCount)

    return CreateRef<VertexBuffer>(driver, std::move(*this));
}


VertexBuffer::VertexBuffer(RHIDriver& driver, Builder&& builder)
	: m_Desc(std::move(builder.m_Desc))
{
    m_Desc.mAttributes[BONE_INDICES].flags |= Attribute::FLAG_INTEGER_TARGET;

    m_VBIHandle = driver.CreateVertexBufferInfo(m_Desc.mBufferCount, m_Desc.mDeclaredAttributes.count(), m_Desc.mAttributes);

    m_Handle = driver.CreateVertexBuffer(m_Desc.mVertexCount, m_VBIHandle);

    // calculate buffer sizes
    size_t bufferSizes[MAX_VERTEX_BUFFER_COUNT] = {};
    #pragma nounroll
    for (size_t i = 0, n = m_Desc.mAttributes.size(); i < n; ++i) {
        if (m_Desc.mDeclaredAttributes[i]) {
            const uint32_t offset = m_Desc.mAttributes[i].offset;
            const uint8_t stride = m_Desc.mAttributes[i].stride;
            const uint8_t slot = m_Desc.mAttributes[i].buffer;
            const size_t end = offset + m_Desc.mVertexCount * stride;
            if (slot != Attribute::BUFFER_UNUSED) {
                ASSERT(slot < MAX_VERTEX_BUFFER_COUNT);
                bufferSizes[slot] = std::max(bufferSizes[slot], end);
            }
        }
    }

    if (!m_Desc.mBufferObjectsEnabled) {
        // If buffer objects are not enabled at the API level, then we create them internally.
        #pragma nounroll
        for (size_t index = 0; index < MAX_VERTEX_BUFFER_COUNT; ++index) {
            size_t const i = m_Desc.mAttributes[index].buffer;
            if (i != Attribute::BUFFER_UNUSED) {
                ASSERT(bufferSizes[i] > 0);
                if (!m_BufferObjects[i]) {
                    BufferObjectHandle bo = driver.CreateBufferObject(bufferSizes[i],
                            BufferObjectBinding::VERTEX, BufferUsage::STATIC);
                    driver.setVertexBufferObject(m_Handle, i, bo);
                    m_BufferObjects[i] = bo;
                }
            }
        }
    }
}

VertexBuffer::~VertexBuffer()
{
	auto& driver = gEngine->GetDriver();
	if (m_Handle)
		driver.DestroyVertexBuffer(m_Handle);
	if (m_VBIHandle)
		driver.DestroyVertexBufferInfo(m_VBIHandle);
	for (auto& boh : m_BufferObjects)
	{
		if (boh)
			driver.DestroyBuffer(boh);
	}
}

void VertexBuffer::setBufferAt(RHIDriver& driver, uint8_t const bufferIndex, BufferDescriptor&& buffer, uint32_t const byteOffset) {
    ASSERT(!m_Desc.mBufferObjectsEnabled);
    if (bufferIndex < m_Desc.mBufferCount) {
        ASSERT(m_BufferObjects[bufferIndex]);
        driver.updateBufferObject(m_BufferObjects[bufferIndex],
               std::move(buffer), byteOffset);
    } else {
        ASSERT(bufferIndex < m_Desc.mBufferCount)
    }
}

void VertexBuffer::setBufferObjectAt(RHIDriver& driver, uint8_t const bufferIndex, Handle<HwBufferObject> bufferObject) {
    ASSERT(m_Desc.mBufferObjectsEnabled);
    // ASSERT(bufferObject->getBindingType() == BufferObjectBinding::VERTEX)
    if (bufferIndex < m_Desc.mBufferCount) {
        auto hwBufferObject = bufferObject;
        driver.setVertexBufferObject(m_Handle, bufferIndex, hwBufferObject);
        // store handle to recreate VertexBuffer in the case extra bone indices and weights definition
        // used only in buffer object mode
        m_BufferObjects[bufferIndex] = hwBufferObject;
    } else {
        ASSERT(bufferIndex < m_Desc.mBufferCount)
    }
}

} // namespace RHI
