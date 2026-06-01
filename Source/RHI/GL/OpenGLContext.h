#pragma once
#include <functional>
#include <map>
#include <optional>
#include <glm/glm.hpp>
#include <glad/glad.h>

#include "Common/Handle.h"
#include "RHI/DriverEnums.h"

namespace RHI
{
    struct HwVertexBuffer;
    struct SamplerParams;

    class OpenGLContext
    {
        friend class GLDriver;
    public:
        OpenGLContext() noexcept;

        static constexpr const size_t MAX_TEXTURE_UNIT_COUNT = 62;
        static constexpr const size_t DUMMY_TEXTURE_BINDING = 7; // highest binding guaranteed to work with ES2
        static constexpr const size_t MAX_BUFFER_BINDINGS = 32;

        struct RenderPrimitive
        {
            static_assert(MAX_VERTEX_ATTRIBUTE_COUNT <= 16);

            GLuint vao[2];											// 8
            GLuint elementArray = 0;                                // 4
            GLenum indicesType = 0;                                 // 4

            // The optional 32-bit handle to a GLVertexBuffer is necessary only if the referenced
            // VertexBuffer supports buffer objects. If this is zero, then the VBO handles array is
            // immutable.
            Handle<HwVertexBuffer> vertexBufferWithObjects;         // 4

            mutable uint16_t vertexAttribArray;						// 2

            uint8_t reserved[2] = {};                               // 2

            // if this differs from vertexBufferWithObjects->bufferObjectsVersion, this VAO needs to
            // be updated (see OpenGLDriver::updateVertexArrayObject())
            uint8_t vertexBufferVersion = 0;                        // 1

            // if this differs from OpenGLContext::state.age, this VAO needs to
            // be updated (see OpenGLDriver::updateVertexArrayObject())
            uint8_t stateVersion = 0;                               // 1

            // If this differs from OpenGLContext::state.age, this VAO's name needs to be updated.
            // See OpenGLContext::bindVertexArray()
            uint8_t nameVersion = 0;                                // 1

            // Size in bytes of indices in the index buffer (1 or 2)
            uint8_t indicesShift = 0;                                // 1

            GLenum getIndicesType() const noexcept {
                return indicesType;
            }
        };

        constexpr        inline size_t getIndexForCap(GLenum cap) noexcept;
        constexpr static inline size_t getIndexForBufferTarget(GLenum target) noexcept;

        void resetState() noexcept;
        void setDefaultState() const noexcept;
        GLuint getSamplerSlow(SamplerParams params) const noexcept;

        inline GLuint getSampler(SamplerParams sp) const noexcept {
            ASSERT(!sp.padding0);
            ASSERT(!sp.padding1);
            ASSERT(!sp.padding2);
            auto& samplerMap = mSamplerMap;
            auto pos = samplerMap.find(sp);
            if (pos == samplerMap.end()) {
                return getSamplerSlow(sp);
            }
            return pos->second;
        }

        inline void useProgram(GLuint program) noexcept;

        void pixelStore(GLenum, GLint) noexcept;
        inline void activeTexture(GLuint unit) noexcept;
        void bindTexture(GLuint unit, GLuint target, GLuint texId, bool external) noexcept;

        void unbindTexture(GLenum target, GLuint id) noexcept;
        void unbindTextureUnit(GLuint unit) noexcept;
        inline void bindVertexArray(RenderPrimitive const* p) noexcept;
        inline void bindSampler(GLuint unit, GLuint sampler) noexcept;
        void unbindSampler(GLuint sampler) noexcept;

        void bindBuffer(GLenum target, GLuint buffer) noexcept;
        void bindBufferRange(GLenum target, GLuint index, GLuint buffer,
                GLintptr offset, GLsizeiptr size) noexcept;

        GLuint bindFramebuffer(GLenum target, GLuint buffer) noexcept;
        void unbindFramebuffer(GLenum target) noexcept;

        inline void enableVertexAttribArray(RenderPrimitive const* rp, GLuint index) noexcept;
        inline void disableVertexAttribArray(RenderPrimitive const* rp, GLuint index) noexcept;
        inline void enable(GLenum cap) noexcept;
        inline void disable(GLenum cap) noexcept;
        inline void frontFace(GLenum mode) noexcept;
        inline void cullFace(GLenum mode) noexcept;
        inline void blendEquation(GLenum modeRGB, GLenum modeA) noexcept;
        inline void blendFunction(GLenum srcRGB, GLenum srcA, GLenum dstRGB, GLenum dstA) noexcept;
        inline void colorMask(GLboolean flag) noexcept;
        inline void depthMask(GLboolean flag) noexcept;
        inline void depthFunc(GLenum func) noexcept;
        inline void stencilFuncSeparate(GLenum funcFront, GLint refFront, GLuint maskFront,
                GLenum funcBack, GLint refBack, GLuint maskBack) noexcept;
        inline void stencilOpSeparate(GLenum sfailFront, GLenum dpfailFront, GLenum dppassFront,
                GLenum sfailBack, GLenum dpfailBack, GLenum dppassBack) noexcept;
        inline void stencilMaskSeparate(GLuint maskFront, GLuint maskBack) noexcept;
        inline void polygonOffset(GLfloat factor, GLfloat units) noexcept;

        inline void setScissor(GLint left, GLint bottom, GLsizei width, GLsizei height) noexcept;
        inline void viewport(GLint left, GLint bottom, GLsizei width, GLsizei height) noexcept;
        inline void depthRange(GLclampf near, GLclampf far) noexcept;

        void deleteBuffer(GLuint buffer, GLenum target) noexcept;
        void deleteVertexArray(GLuint vao) noexcept;

        void destroyWithContext(size_t index, std::function<void(OpenGLContext&)> const& closure) noexcept;

        struct State {
            State() noexcept = default;
            // make sure we don't copy this state by accident
            State(State const& rhs) = delete;
            State(State&& rhs) noexcept = delete;
            State& operator=(State const& rhs) = delete;
            State& operator=(State&& rhs) noexcept = delete;

            GLint major = 0;
            GLint minor = 0;

            char const* vendor = nullptr;
            char const* renderer = nullptr;
            char const* version = nullptr;
            char const* shader = nullptr;

            GLuint draw_fbo = 0;
            GLuint read_fbo = 0;

            struct {
                GLuint use = 0;
            } program;

            struct {
                RenderPrimitive* p = nullptr;
            } vao;

            struct {
                GLenum frontFace            = GL_CCW;
                GLenum cullFace             = GL_BACK;
                GLenum blendEquationRGB     = GL_FUNC_ADD;
                GLenum blendEquationA       = GL_FUNC_ADD;
                GLenum blendFunctionSrcRGB  = GL_ONE;
                GLenum blendFunctionSrcA    = GL_ONE;
                GLenum blendFunctionDstRGB  = GL_ZERO;
                GLenum blendFunctionDstA    = GL_ZERO;
                GLboolean colorMask         = GL_TRUE;
                GLboolean depthMask         = GL_TRUE;
                GLenum depthFunc            = GL_LESS;
            } raster;

            struct {
                struct StencilFunc {
                    GLenum func             = GL_ALWAYS;
                    GLint ref               = 0;
                    GLuint mask             = ~GLuint(0);
                    bool operator != (StencilFunc const& rhs) const noexcept {
                        return func != rhs.func || ref != rhs.ref || mask != rhs.mask;
                    }
                };
                struct StencilOp {
                    GLenum sfail            = GL_KEEP;
                    GLenum dpfail           = GL_KEEP;
                    GLenum dppass           = GL_KEEP;
                    bool operator != (StencilOp const& rhs) const noexcept {
                        return sfail != rhs.sfail || dpfail != rhs.dpfail || dppass != rhs.dppass;
                    }
                };
                struct {
                    StencilFunc func;
                    StencilOp op;
                    GLuint stencilMask      = ~GLuint(0);
                } front, back;
            } stencil;

            struct PolygonOffset {
                GLfloat factor = 0;
                GLfloat units = 0;
                bool operator != (PolygonOffset const& rhs) const noexcept {
                    return factor != rhs.factor || units != rhs.units;
                }
            } polygonOffset;

            struct {
                uint32_t caps;
            } enables;

            struct {
                struct {
                    struct {
                        GLuint name = 0;
                        GLintptr offset = 0;
                        GLsizeiptr size = 0;
                    } buffers[MAX_BUFFER_BINDINGS];
                } targets[3];   // there are only 3 indexed buffer targets
                GLuint genericBinding[7] = {};
            } buffers;

            struct {
                GLuint active = 0;      // zero-based
                struct {
                    GLuint sampler = 0;
                    GLuint target = 0;
                    GLuint id = 0;
                } units[MAX_TEXTURE_UNIT_COUNT];
            } textures;

            struct {
                GLint row_length = 0;
                GLint alignment = 4;
            } unpack;

            struct {
                GLint alignment = 4;
            } pack;

            struct {
                glm::ivec4 scissor { 0 };
                glm::ivec4 viewport { 0 };
                glm::vec2 depthRange { 0.0f, 1.0f };
            } window;
            uint8_t age = 0;
        } state;

        struct Gets {
            GLfloat max_anisotropy;
            GLint max_combined_texture_image_units;
            GLint max_draw_buffers;
            GLint max_renderbuffer_size;
            GLint max_samples;
            GLint max_texture_image_units;
            GLint max_transform_feedback_separate_attribs;
            GLint max_uniform_block_size;
            GLint max_uniform_buffer_bindings;
            GLint num_program_binary_formats;
            GLint uniform_buffer_offset_alignment;
        } gets = {};

        template <typename T, typename F>
        static inline void update_state(T& state, T const& expected, F functor, bool force = false) noexcept {
            if (UTILS_UNLIKELY(force || state != expected)) {
                state = expected;
                functor();
            }
        }

    private:
        void bindFramebufferResolved(GLenum target, GLuint buffer) noexcept;

    private:
        uint8_t  m_ContextIndex;
        std::optional<GLuint> m_DefaultFbo[2];
        RenderPrimitive m_DefaultVAO;
        mutable std::map<SamplerParams, GLuint, SamplerParams::LessThan> mSamplerMap;
    };

void OpenGLContext::polygonOffset(GLfloat factor, GLfloat units) noexcept {
    update_state(state.polygonOffset, { factor, units }, [&]() {
        if (factor != 0 || units != 0) {
            glPolygonOffset(factor, units);
            enable(GL_POLYGON_OFFSET_FILL);
        } else {
            disable(GL_POLYGON_OFFSET_FILL);
        }
    });
}

void OpenGLContext::setScissor(GLint left, GLint bottom, GLsizei width, GLsizei height) noexcept {
    glm::ivec4 const scissor(left, bottom, width, height);
    update_state(state.window.scissor, scissor, [&]() {
        glScissor(left, bottom, width, height);
    });
}

void OpenGLContext::viewport(GLint left, GLint bottom, GLsizei width, GLsizei height) noexcept {
    glm::ivec4 const viewport(left, bottom, width, height);
    update_state(state.window.viewport, viewport, [&]() {
        glViewport(left, bottom, width, height);
    });
}

void OpenGLContext::depthRange(GLclampf near, GLclampf far) noexcept {
    glm::vec2 const depthRange(near, far);
    update_state(state.window.depthRange, depthRange, [&]() {
        glDepthRangef(near, far);
    });
}

constexpr size_t OpenGLContext::getIndexForCap(GLenum cap) noexcept { //NOLINT
    size_t index = 0;
    switch (cap) {
    case GL_BLEND:                          index =  0; break;
    case GL_CULL_FACE:                      index =  1; break;
    case GL_SCISSOR_TEST:                   index =  2; break;
    case GL_DEPTH_TEST:                     index =  3; break;
    case GL_STENCIL_TEST:                   index =  4; break;
    case GL_DITHER:                         index =  5; break;
    case GL_SAMPLE_ALPHA_TO_COVERAGE:       index =  6; break;
    case GL_SAMPLE_COVERAGE:                index =  7; break;
    case GL_POLYGON_OFFSET_FILL:            index =  8; break;
#ifdef GL_ARB_seamless_cube_map
    case GL_TEXTURE_CUBE_MAP_SEAMLESS:      index =  9; break;
#endif
#ifdef BACKEND_OPENGL_VERSION_GL
    case GL_PROGRAM_POINT_SIZE:             index = 10; break;
#endif
    case GL_DEPTH_CLAMP:                    index = 11; break;
    default: break;
    }
    ASSERT(index < 16);
    return index;
}

void OpenGLContext::useProgram(GLuint program) noexcept {
    update_state(state.program.use, program, [&]() {
        glUseProgram(program);
    });
}

void OpenGLContext::enableVertexAttribArray(RenderPrimitive const* rp, GLuint index) noexcept {
    ASSERT(rp);
    ASSERT(index < 16);
    bool const force = rp->stateVersion != state.age;
    if ((force || !(rp->vertexAttribArray & (1u <<index)))) {
        rp->vertexAttribArray |= (1u << index);
        glEnableVertexAttribArray(index);
    }
}

void OpenGLContext::disableVertexAttribArray(RenderPrimitive const* rp, GLuint index) noexcept {
    ASSERT(rp);
    ASSERT(index < 16);
    bool const force = rp->stateVersion != state.age;
    if (force || rp->vertexAttribArray & (1u <<index)) {
        rp->vertexAttribArray &= ~(1u << index);
        glDisableVertexAttribArray(index);
    }
}

void OpenGLContext::enable(GLenum cap) noexcept {
    size_t const index = getIndexForCap(cap);
    if (!(state.enables.caps & (1 << index))) {
        state.enables.caps |= (1 << index);
        glEnable(cap);
    }
}

void OpenGLContext::disable(GLenum cap) noexcept {
    size_t const index = getIndexForCap(cap);
    if ((state.enables.caps & (1u << index))) {
        state.enables.caps &= ~(1u << index);
        glDisable(cap);
    }
}

void OpenGLContext::frontFace(GLenum mode) noexcept {
    update_state(state.raster.frontFace, mode, [&]() {
        glFrontFace(mode);
    });
}

void OpenGLContext::cullFace(GLenum mode) noexcept {
    update_state(state.raster.cullFace, mode, [&]() {
        glCullFace(mode);
    });
}

void OpenGLContext::blendEquation(GLenum modeRGB, GLenum modeA) noexcept {
    if ((
            state.raster.blendEquationRGB != modeRGB || state.raster.blendEquationA != modeA)) {
        state.raster.blendEquationRGB = modeRGB;
        state.raster.blendEquationA   = modeA;
        glBlendEquationSeparate(modeRGB, modeA);
    }
}

void OpenGLContext::blendFunction(GLenum srcRGB, GLenum srcA, GLenum dstRGB, GLenum dstA) noexcept {
    if ((
            state.raster.blendFunctionSrcRGB != srcRGB ||
            state.raster.blendFunctionSrcA != srcA ||
            state.raster.blendFunctionDstRGB != dstRGB ||
            state.raster.blendFunctionDstA != dstA)) {
        state.raster.blendFunctionSrcRGB = srcRGB;
        state.raster.blendFunctionSrcA = srcA;
        state.raster.blendFunctionDstRGB = dstRGB;
        state.raster.blendFunctionDstA = dstA;
        glBlendFuncSeparate(srcRGB, dstRGB, srcA, dstA);
    }
}

void OpenGLContext::colorMask(GLboolean flag) noexcept {
    update_state(state.raster.colorMask, flag, [&]() {
        glColorMask(flag, flag, flag, flag);
    });
}
void OpenGLContext::depthMask(GLboolean flag) noexcept {
    update_state(state.raster.depthMask, flag, [&]() {
        glDepthMask(flag);
    });
}

void OpenGLContext::depthFunc(GLenum func) noexcept {
    update_state(state.raster.depthFunc, func, [&]() {
        glDepthFunc(func);
    });
}

void OpenGLContext::stencilFuncSeparate(GLenum funcFront, GLint refFront, GLuint maskFront,
        GLenum funcBack, GLint refBack, GLuint maskBack) noexcept {
    update_state(state.stencil.front.func, {funcFront, refFront, maskFront}, [&]() {
        glStencilFuncSeparate(GL_FRONT, funcFront, refFront, maskFront);
    });
    update_state(state.stencil.back.func, {funcBack, refBack, maskBack}, [&]() {
        glStencilFuncSeparate(GL_BACK, funcBack, refBack, maskBack);
    });
}

void OpenGLContext::stencilOpSeparate(GLenum sfailFront, GLenum dpfailFront, GLenum dppassFront,
        GLenum sfailBack, GLenum dpfailBack, GLenum dppassBack) noexcept {
    update_state(state.stencil.front.op, {sfailFront, dpfailFront, dppassFront}, [&]() {
        glStencilOpSeparate(GL_FRONT, sfailFront, dpfailFront, dppassFront);
    });
    update_state(state.stencil.back.op, {sfailBack, dpfailBack, dppassBack}, [&]() {
        glStencilOpSeparate(GL_BACK, sfailBack, dpfailBack, dppassBack);
    });
}

void OpenGLContext::stencilMaskSeparate(GLuint maskFront, GLuint maskBack) noexcept {
    update_state(state.stencil.front.stencilMask, maskFront, [&]() {
        glStencilMaskSeparate(GL_FRONT, maskFront);
    });
    update_state(state.stencil.back.stencilMask, maskBack, [&]() {
        glStencilMaskSeparate(GL_BACK, maskBack);
    });
}
}
