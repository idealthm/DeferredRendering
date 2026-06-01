#pragma once
#include "Common/Core.h"
#include "Common/Handle.h"
#include "DriverEnums.h"
#include "BufferDescriptor.h"
#include "BufferLayout.h"
#include "Common/Utils.h"
#include "Common/Material/MaterialCommon.h"
#include "Common/Utils/Bitset.h"

namespace RHI
{
	class RHIDriver;
}

namespace RHI
{

using AttributeBitset = Util::bitset32;
	
struct VertexBufferDesc {
	struct AttributeData : Attribute {
		AttributeData() { type = ElementType::FLOAT4; 
			static_assert(sizeof(Attribute) == sizeof(AttributeData),
					"Attribute and Builder::Attribute must match");
		}
	};
	std::string mName;
	AttributeArray mAttributes{};
	AttributeBitset mDeclaredAttributes;
	uint32_t mVertexCount = 0;
	uint8_t mBufferCount = 0;
	bool mBufferObjectsEnabled = false;
};

class VertexBuffer
{
public:

	class Builder {
        friend struct VertexBufferDesc;
    public:
        Builder() noexcept = default;
        Builder(Builder const& rhs) noexcept = default;
        Builder(Builder&& rhs) noexcept = default;
        ~Builder() noexcept = default;
        Builder& operator=(Builder const& rhs) noexcept = default;
        Builder& operator=(Builder&& rhs) noexcept = default;

        Builder& bufferCount(uint8_t bufferCount) noexcept;

        Builder& vertexCount(uint32_t vertexCount) noexcept;

        Builder& enableBufferObjects(bool enabled = true) noexcept;

        Builder& attribute(VertexAttribute attribute, uint8_t bufferIndex,
                ElementType attributeType,
                uint32_t byteOffset = 0, uint8_t byteStride = 0) noexcept;

        Builder& normalized(VertexAttribute attribute, bool normalize = true) noexcept;

        Builder& name(const std::string& name) noexcept;

        Ref<VertexBuffer> build(RHIDriver& driver);

    private:
		VertexBufferDesc m_Desc;
        friend class VertexBuffer;
    };

	VertexBuffer(RHIDriver& driver, Builder&& builder);
	virtual ~VertexBuffer();

	bool IsValid() const { return !!m_Handle; }
	Handle<HwVertexBuffer> GetHandle() const { return m_Handle; }
	Handle<HwVertexBufferInfo> GetVertexBufferInfoHandle() const { return m_VBIHandle; }
	void setBufferAt(RHIDriver& driver, uint8_t bufferIndex, BufferDescriptor&& buffer, uint32_t byteOffset = 0);
	void setBufferObjectAt(RHIDriver& driver, uint8_t bufferIndex, Handle<RHI::HwBufferObject> bufferObject);
	const VertexBufferDesc& GetDesc() const { return m_Desc; }

private:
	VertexBufferDesc m_Desc;
	Handle<HwVertexBuffer> m_Handle;
	Handle<HwVertexBufferInfo> m_VBIHandle;
	std::array<Handle<RHI::HwBufferObject>, MAX_VERTEX_BUFFER_COUNT> m_BufferObjects;
};

} // namespace RHI
