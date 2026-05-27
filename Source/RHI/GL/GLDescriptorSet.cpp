#include "GLDescriptorSet.h"

#include <variant>

#include "GLDescriptorSetLayout.h"
#include "GLDriver.h"
#include "GLHelper.h"
#include "GLProgram.h"
#include "Common/Utils/Bitset.h"

namespace RHI
{
    
GLDescriptorSet::GLDescriptorSet(OpenGLContext& gl, Handle<HwDescriptorSetLayout> dslh,
        GLDescriptorSetLayout const* layout) noexcept
        : descriptors(layout->maxDescriptorBinding + 1),
          dslh(std::move(dslh)) {

    // We have allocated enough storage for all descriptors. Now allocate the empty descriptor
    // themselves.
    for (auto const& entry : layout->bindings) {
        size_t const index = entry.binding;

        // now we'll initialize the alternative for each way we can handle this descriptor.
        auto& desc = descriptors[index].desc;
        switch (entry.type) {
            case DescriptorType::UNIFORM_BUFFER: {
                // A uniform buffer can have dynamic offsets or not and have special handling for
                // ES2 (where we need to emulate it). That's four alternatives.
                bool const dynamicOffset = entry.flags == DescriptorFlags::DYNAMIC_OFFSET;
                dynamicBuffers.set(index, dynamicOffset);
                
                auto const type = RHI_Internal::GetGLBufferBinding(BufferObjectBinding::UNIFORM);
                if (dynamicOffset) {
                    dynamicBufferCount++;
                    desc.emplace<DynamicBuffer>(type);
                } else {
                    desc.emplace<Buffer>(type);
                }
                break;
            }
            case DescriptorType::SHADER_STORAGE_BUFFER: {
                // shader storage buffers are not supported on ES2, So that's two alternatives.
                bool const dynamicOffset = entry.flags == DescriptorFlags::DYNAMIC_OFFSET;
                dynamicBuffers.set(index, dynamicOffset);
                auto const type = RHI_Internal::GetGLBufferBinding(BufferObjectBinding::SHADER_STORAGE);
                if (dynamicOffset) {
                    dynamicBufferCount++;
                    desc.emplace<DynamicBuffer>(type);
                } else {
                    desc.emplace<Buffer>(type);
                }
                break;
            }
            case DescriptorType::SAMPLER:
                desc.emplace<Sampler>();
                break;
            case DescriptorType::INPUT_ATTACHMENT:
                break;
        }
    }
}

void GLDescriptorSet::update(OpenGLContext& gl, uint8_t binding, GLBufferObject* bo, size_t offset, size_t size) noexcept {
    ASSERT(binding < descriptors.size());
    std::cout << "update buffer " << (uint32_t)binding << " " << " " << offset << " " << size << std::endl;
    std::visit([=](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, Buffer> || std::is_same_v<T, DynamicBuffer>) {
            ASSERT(arg.target != 0);
            arg.id = bo ? bo->gl.id : 0;
            arg.offset = uint32_t(offset);
            arg.size = uint32_t(size);
            ASSERT(arg.id || (!arg.size && !offset));
        } else {
            // API usage error. User asked to update the wrong type of descriptor.
            ASSERT(0);
        }
    }, descriptors[binding].desc);
}

void GLDescriptorSet::update(OpenGLContext& gl, uint8_t binding, GLTexture* t, SamplerParams params) noexcept {
    std::cout << "update Texture " << (uint32_t)binding << " " << std::endl;
    ASSERT(binding < descriptors.size());
    std::visit([=, &gl](auto&& arg) mutable {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, Sampler> ||
                      std::is_same_v<T, SamplerWithAnisotropyWorkaround>) {
            // GLES3.x specification forbids depth textures to be filtered.
            if (t && isDepthFormat(t->format)
                    && params.compareMode == SamplerCompareMode::NONE) {
                params.filterMag = SamplerMagFilter::Nearest;
                switch (params.filterMin) {
                    case SamplerMinFilter::Linear:
                        params.filterMin = SamplerMinFilter::Nearest;
                        break;
                    case SamplerMinFilter::LinearMipmapNearest:
                    case SamplerMinFilter::NearestMipmapLinear:
                    case SamplerMinFilter::LinearMipmapLinear:
                        params.filterMin = SamplerMinFilter::NearestMipmapNearest;
                        break;
                    default:
                        break;
                }
            }

            arg.target = t ? t->gl.target : 0;
            arg.id = t ? t->gl.id : 0;
            if constexpr (std::is_same_v<T, Sampler> ||
                          std::is_same_v<T, SamplerWithAnisotropyWorkaround>) {
                if constexpr (std::is_same_v<T, SamplerWithAnisotropyWorkaround>) {
                    arg.anisotropy = float(1u << params.anisotropyLog2);
                }
                if (t) {
                    arg.ref = t->ref;
                    arg.baseLevel = t->gl.baseLevel;
                    arg.maxLevel = t->gl.maxLevel;
                }
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
                arg.sampler = gl.getSampler(params);
#else
                (void)gl;
#endif
            } else {
                arg.params = params;
            }
        } else {
            ASSERT(0);
        }
    }, descriptors[binding].desc);
}

template<typename T>
void GLDescriptorSet::updateTextureView(OpenGLContext& gl,
        HandleAllocatorGL& handleAllocator, GLuint unit, T const& desc) noexcept {
    // The common case is that we don't have a ref handle (we only have one if
    // the texture ever had a View on it).
    ASSERT(desc.ref);
    GLTextureRef* const ref = handleAllocator.handle_cast<GLTextureRef*>(desc.ref);
    if (UTILS_UNLIKELY((desc.baseLevel != ref->baseLevel || desc.maxLevel != ref->maxLevel))) {
        // If we have views, then it's still uncommon that we'll switch often
        // handle the case where we reset to the original texture
        GLint baseLevel = GLint(desc.baseLevel); // NOLINT(*-signed-char-misuse)
        GLint maxLevel = GLint(desc.maxLevel); // NOLINT(*-signed-char-misuse)
        if (baseLevel > maxLevel) {
            baseLevel = 0;
            maxLevel = 1000; // per OpenGL spec
        }
        // that is very unfortunate that we have to call activeTexture here
        gl.activeTexture(unit);
        glTexParameteri(desc.target, GL_TEXTURE_BASE_LEVEL, baseLevel);
        glTexParameteri(desc.target, GL_TEXTURE_MAX_LEVEL,  maxLevel);
        ref->baseLevel = desc.baseLevel;
        ref->maxLevel = desc.maxLevel;
    }
}

void GLDescriptorSet::bind(OpenGLContext& gl, HandleAllocatorGL& handleAllocator,
        GLProgram const& p,
        uint8_t set, uint32_t const* offsets, bool offsetsOnly) const noexcept {
    // TODO: check that offsets is sized correctly
    size_t dynamicOffsetIndex = 0;

    Util::bitset64 activeDescriptorBindings = p.getActiveDescriptors(set);
    if (offsetsOnly) {
        activeDescriptorBindings &= dynamicBuffers;
    }

    // loop only over the active indices for this program
    activeDescriptorBindings.forEachSetBit(
            [this,&gl, &handleAllocator, &p, set, offsets, &dynamicOffsetIndex]
            (size_t binding) {

        // This would fail here if we're trying to set a descriptor that doesn't exist in the
        // program. In other words, a mismatch between the program's layout and this descriptor-set.
        ASSERT(binding < descriptors.size());

        auto const& entry = descriptors[binding];
        std::visit(
                [&gl, &handleAllocator, &p, &dynamicOffsetIndex, set, binding, offsets]
                (auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, Buffer>) {
                GLuint const bindingPoint = p.getBufferBinding(set, binding);
                GLintptr const offset = arg.offset;
                ASSERT(arg.id || (!arg.size && !offset));
                gl.bindBufferRange(arg.target, bindingPoint, arg.id, offset, arg.size);
            } else if constexpr (std::is_same_v<T, DynamicBuffer>) {
                GLuint const bindingPoint = p.getBufferBinding(set, binding);
                GLintptr const offset = arg.offset + offsets[dynamicOffsetIndex++];
                ASSERT(arg.id || (!arg.size && !offset));
                gl.bindBufferRange(arg.target, bindingPoint, arg.id, offset, arg.size);
            } else if constexpr (std::is_same_v<T, Sampler>) {
                GLuint const unit = p.getTextureUnit(set, binding);
                if (arg.target) {
                    gl.bindTexture(unit, arg.target, arg.id, arg.external);
                    gl.bindSampler(unit, arg.sampler);
                    if (arg.ref) {
                        updateTextureView(gl, handleAllocator, unit, arg);
                    }
                } else {
                    gl.unbindTextureUnit(unit);
                }
            } else if constexpr (std::is_same_v<T, SamplerWithAnisotropyWorkaround>) {
                GLuint const unit = p.getTextureUnit(set, binding);
                if (arg.target) {
                    gl.bindTexture(unit, arg.target, arg.id, arg.external);
                    gl.bindSampler(unit, arg.sampler);
                    if (arg.ref) {
                        updateTextureView(gl, handleAllocator, unit, arg);
                    }
#if defined(GL_EXT_texture_filter_anisotropic)
                    // Driver claims to support anisotropic filtering, but it fails when set on
                    // the sampler, we have to set it on the texture instead.
                    glTexParameterf(arg.target, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                            std::min(gl.gets.max_anisotropy, float(arg.anisotropy)));
#endif
                } else {
                    gl.unbindTextureUnit(unit);
                }
            }
        }, entry.desc);
    });
}

void GLDescriptorSet::validate(HandleAllocatorGL& allocator,
        Handle<HwDescriptorSetLayout> pipelineLayout) const {

    if ((dslh != pipelineLayout)) {
        auto* const dsl = allocator.handle_cast < GLDescriptorSetLayout const * > (dslh);
        auto* const cur = allocator.handle_cast < GLDescriptorSetLayout const * > (pipelineLayout);

        bool const pipelineLayoutMatchesDescriptorSetLayout = std::equal(
                dsl->bindings.begin(), dsl->bindings.end(),
                cur->bindings.begin(),
                [](DescriptorSetLayoutBinding const& lhs,
                        DescriptorSetLayoutBinding const& rhs) {
                    return lhs.type == rhs.type &&
                           lhs.stageFlags == rhs.stageFlags &&
                           lhs.binding == rhs.binding &&
                           lhs.flags == rhs.flags &&
                           lhs.count == rhs.count;
                });

        ASSERT(pipelineLayoutMatchesDescriptorSetLayout);
    }
}
}
    