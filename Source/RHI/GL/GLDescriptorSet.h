
#include <variant>
#include <array>

#include "GLDriver.h"
#include "GLHelper.h"
#include "Common/Utils/Bitset.h"
#include "RHI/RHIDriver.h"

namespace RHI
{

	struct GLDescriptorSet : public HwDescriptorSet {

    using HwDescriptorSet::HwDescriptorSet;

    GLDescriptorSet(OpenGLContext& gl, Handle<HwDescriptorSetLayout> dslh,
            GLDescriptorSetLayout const* layout) noexcept;

    // update a buffer descriptor in the set
    void update(OpenGLContext& gl,uint8_t binding, GLBufferObject* bo, size_t offset, size_t size) noexcept;

    // update a sampler descriptor in the set
    void update(OpenGLContext& gl, uint8_t binding, GLTexture* t, SamplerParams params) noexcept;

    // conceptually bind the set to the command buffer
    void bind(
            OpenGLContext& gl,
            HandleAllocatorGL& handleAllocator,
            GLProgram const& p,
            uint8_t set, uint32_t const* offsets, bool offsetsOnly) const noexcept;

    uint32_t getDynamicBufferCount() const noexcept {
        return dynamicBufferCount;
    }

    void validate(HandleAllocatorGL& allocator, Handle<HwDescriptorSetLayout> pipelineLayout) const;

private:
    // a Buffer Descriptor such as SSBO or UBO with static offset
    struct Buffer {
        // Workaround: we cannot define the following as Buffer() = default because one of our
        // clients has their compiler set up where such declaration (possibly coupled with explicit)
        // will be considered a deleted constructor.
        Buffer() {}

        explicit Buffer(GLenum target) noexcept : target(target) {}
        GLenum target;                          // 4
        GLuint id = 0;                          // 4
        uint32_t offset = 0;                    // 4
        uint32_t size = 0;                      // 4
    };

    // a Buffer Descriptor such as SSBO or UBO with dynamic offset
    struct DynamicBuffer {
        DynamicBuffer() = default;
        explicit DynamicBuffer(GLenum target) noexcept : target(target) { }
        GLenum target;                          // 4
        GLuint id = 0;                          // 4
        uint32_t offset = 0;                    // 4
        uint32_t size = 0;                      // 4
    };

    // A sampler descriptor
    struct Sampler {
        uint16_t target;                        // 2 (GLenum)
        bool external = false;                  // 1
        bool reserved = false;                  // 1
        GLuint id = 0;                          // 4
        GLuint sampler = 0;                     // 4
        Handle<GLTextureRef> ref;               // 4
        int8_t baseLevel = 0x7f;                // 1
        int8_t maxLevel = -1;                   // 1
    };

    struct SamplerWithAnisotropyWorkaround {
        uint16_t target;                        // 2 (GLenum)
        bool external = false;                  // 1
        bool reserved = false;                  // 1
        GLuint id = 0;                          // 4
        GLuint sampler = 0;                     // 4
        Handle<GLTextureRef> ref;               // 4
        float anisotropy = 1.0f;                // 4
        int8_t baseLevel = 0x7f;                // 1
        int8_t maxLevel = -1;                   // 1
    };

    struct Descriptor {
        std::variant<
            Buffer,
            DynamicBuffer,
            Sampler,
            SamplerWithAnisotropyWorkaround
        > desc;
    };
    static_assert(sizeof(Descriptor) <= 32);

    template<typename T>
    static void updateTextureView(OpenGLContext& gl,
            HandleAllocatorGL& handleAllocator, GLuint unit, T const& desc) noexcept;

    std::vector<Descriptor> descriptors;     // 16
    Util::bitset64 dynamicBuffers;                         // 8
    Handle<HwDescriptorSetLayout> dslh;                         // 4
    uint8_t dynamicBufferCount = 0;                         // 1
};
}
