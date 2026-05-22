#include "OpenGLContext.h"
#include "GLHelper.h"

namespace RHI
{

OpenGLContext::OpenGLContext() noexcept
{

    state.vao.p = &m_DefaultVAO;

    // These queries work with all GL/GLES versions!
    state.vendor   = (char const*)glGetString(GL_VENDOR);
    state.renderer = (char const*)glGetString(GL_RENDERER);
    state.version  = (char const*)glGetString(GL_VERSION);
    state.shader   = (char const*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    /*
     * Figure out GL / GLES version, extensions and capabilities we need to
     * determine the feature level
     */

    glGetIntegerv(GL_MAJOR_VERSION, &state.major);
    glGetIntegerv(GL_MINOR_VERSION, &state.minor);

    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE,             &gets.max_renderbuffer_size);
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,           &gets.max_texture_image_units);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,  &gets.max_combined_texture_image_units);

    glGetIntegerv(GL_MAX_DRAW_BUFFERS,
            &gets.max_draw_buffers);
    glGetIntegerv(GL_MAX_SAMPLES,
            &gets.max_samples);
    glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS,
            &gets.max_transform_feedback_separate_attribs);
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE,
            &gets.max_uniform_block_size);
    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS,
            &gets.max_uniform_buffer_bindings);
    glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS,
            &gets.num_program_binary_formats);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT,
            &gets.uniform_buffer_offset_alignment);

    setDefaultState();
}

void OpenGLContext::pixelStore(GLenum pname, GLint param) noexcept {
    GLint* pcur;

    // Note: GL_UNPACK_SKIP_PIXELS, GL_UNPACK_SKIP_ROWS and
    //       GL_PACK_SKIP_PIXELS, GL_PACK_SKIP_ROWS
    // are actually provided as conveniences to the programmer; they provide no functionality
    // that cannot be duplicated at the call site (e.g. glTexImage2D or glReadPixels)

    switch (pname) {
        case GL_PACK_ALIGNMENT:
            pcur = &state.pack.alignment;
            break;
        case GL_UNPACK_ALIGNMENT:
            pcur = &state.unpack.alignment;
            break;
        default:
            goto default_case;
    }

    if (*pcur != param) {
        *pcur = param;
default_case:
        glPixelStorei(pname, param);
    }
}

void OpenGLContext::activeTexture(GLuint unit) noexcept {
    ASSERT(unit < MAX_TEXTURE_UNIT_COUNT);
    update_state(state.textures.active, unit, [&]() {
        glActiveTexture(GL_TEXTURE0 + unit);
    });
}

void OpenGLContext::bindTexture(GLuint unit, GLuint target, GLuint texId, bool external) noexcept {
    //  another texture is bound to the same unit with a different target,
    //  unbind the texture from the current target
    update_state(state.textures.units[unit].target, target, [&]() {
        activeTexture(unit);
        glBindTexture(state.textures.units[unit].target, 0);
    });
    update_state(state.textures.units[unit].id, texId, [&]() {
        activeTexture(unit);
        glBindTexture(target, texId);
    }, external);
}

    
void OpenGLContext::unbindTexture(GLenum target, GLuint texture_id) noexcept {
    // unbind this texture from all the units it might be bound to
    // no need unbind the texture from FBOs because we're not tracking that state (and there is
    // no need to).
    // Never attempt to unbind texture 0. This could happen with external textures w/ streaming if
    // never populated.
    if (texture_id) {
        for (GLuint unit = 0; unit < MAX_TEXTURE_UNIT_COUNT; unit++) {
            if (state.textures.units[unit].id == texture_id) {
                // if this texture is bound, it should be at the same target
                ASSERT(state.textures.units[unit].target == target);
                unbindTextureUnit(unit);
            }
        }
    }
}

void OpenGLContext::unbindTextureUnit(GLuint unit) noexcept {
    update_state(state.textures.units[unit].id, 0u, [&]() {
        activeTexture(unit);
        glBindTexture(state.textures.units[unit].target, 0u);
    });
}

void OpenGLContext::unbindSampler(GLuint sampler) noexcept {
    // unbind this sampler from all the units it might be bound to
    for (GLuint unit = 0; unit < MAX_TEXTURE_UNIT_COUNT; unit++) {
        if (state.textures.units[unit].sampler == sampler) {
            bindSampler(unit, 0);
        }
    }
}

    
void OpenGLContext::bindVertexArray(RenderPrimitive const* p) noexcept {
    RenderPrimitive* vao = p ? const_cast<RenderPrimitive *>(p) : &m_DefaultVAO;
    update_state(state.vao.p, vao, [&]() {

        // See if we need to create a name for this VAO on the fly, this would happen if:
        // - we're not the default VAO, because its name is always 0
        // - our name is 0, this could happen if this VAO was created in the "other" context
        // - the nameVersion is out of date *and* we're on the protected context, in this case:
        //      - the name must be stale from a previous use of this context because we always
        //        destroy the protected context when we're done with it.
        bool const recreateVaoName = p != &m_DefaultVAO &&
                ((vao->vao[m_ContextIndex] == 0) ||
                        (vao->nameVersion != state.age && m_ContextIndex == 1));
        if (recreateVaoName) {
            vao->nameVersion = state.age;
            glGenVertexArrays(1, &vao->vao[m_ContextIndex]);
        }

        glBindVertexArray(vao->vao[m_ContextIndex]);
        // update GL_ELEMENT_ARRAY_BUFFER, which is updated by glBindVertexArray
        size_t const targetIndex = getIndexForBufferTarget(GL_ELEMENT_ARRAY_BUFFER);
        state.buffers.genericBinding[targetIndex] = vao->elementArray;
    });
}

void OpenGLContext::bindSampler(GLuint unit, GLuint sampler) noexcept {
    ASSERT(unit < MAX_TEXTURE_UNIT_COUNT);
    update_state(state.textures.units[unit].sampler, sampler, [&]() {
        glBindSampler(unit, sampler);
    });
}


void OpenGLContext::bindBuffer(GLenum target, GLuint buffer) noexcept {
    if (target == GL_ELEMENT_ARRAY_BUFFER) {
        size_t const targetIndex = getIndexForBufferTarget(GL_ELEMENT_ARRAY_BUFFER);
        // GL_ELEMENT_ARRAY_BUFFER is a special case, where the currently bound VAO remembers
        // the index buffer, unless there are no VAO bound (see: bindVertexArray)
        ASSERT(state.vao.p);
        if (state.buffers.genericBinding[targetIndex] != buffer
            || ((state.vao.p != &m_DefaultVAO) && (state.vao.p->elementArray != buffer))) {
            state.buffers.genericBinding[targetIndex] = buffer;
            if (state.vao.p != &m_DefaultVAO) {
                state.vao.p->elementArray = buffer;
            }
            glBindBuffer(target, buffer);
            }
    } else {
        size_t const targetIndex = getIndexForBufferTarget(target);
        update_state(state.buffers.genericBinding[targetIndex], buffer, [&]() {
            glBindBuffer(target, buffer);
        });
    }
}
    
void OpenGLContext::bindBufferRange(GLenum target, GLuint index, GLuint buffer,
        GLintptr offset, GLsizeiptr size) noexcept {

    size_t const targetIndex = getIndexForBufferTarget(target);
    // this ALSO sets the generic binding
    ASSERT(targetIndex < sizeof(state.buffers.targets) / sizeof(*state.buffers.targets));
    if (   state.buffers.targets[targetIndex].buffers[index].name != buffer
           || state.buffers.targets[targetIndex].buffers[index].offset != offset
           || state.buffers.targets[targetIndex].buffers[index].size != size) {
        state.buffers.targets[targetIndex].buffers[index].name = buffer;
        state.buffers.targets[targetIndex].buffers[index].offset = offset;
        state.buffers.targets[targetIndex].buffers[index].size = size;
        state.buffers.genericBinding[targetIndex] = buffer;
        glBindBufferRange(target, index, buffer, offset, size);
    }
}


GLuint OpenGLContext::bindFramebuffer(GLenum target, GLuint buffer) noexcept {
    if (buffer == 0) {
        // we're binding the default frame buffer, resolve its actual name
        auto& defaultFboForThisContext = m_DefaultFbo[m_ContextIndex];
        if (!defaultFboForThisContext.has_value()) {
            defaultFboForThisContext = 0;
        }
        buffer = defaultFboForThisContext.value();
    }
    bindFramebufferResolved(target, buffer);
    return buffer;
}

void OpenGLContext::unbindFramebuffer(GLenum target) noexcept {
    bindFramebufferResolved(target, 0);
}


void OpenGLContext::bindFramebufferResolved(GLenum target, GLuint buffer) noexcept {
    switch (target) {
    case GL_FRAMEBUFFER:
        if (state.draw_fbo != buffer || state.read_fbo != buffer) {
            state.draw_fbo = state.read_fbo = buffer;
            glBindFramebuffer(target, buffer);
        }
        break;
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    case GL_DRAW_FRAMEBUFFER:
        if (state.draw_fbo != buffer) {
            state.draw_fbo = buffer;
            glBindFramebuffer(target, buffer);
        }
        break;
    case GL_READ_FRAMEBUFFER:
        if (state.read_fbo != buffer) {
            state.read_fbo = buffer;
            glBindFramebuffer(target, buffer);
        }
        break;
#endif
    default:
        break;
    }
}
    
void OpenGLContext::deleteBuffer(GLuint buffer, GLenum target) noexcept {
    glDeleteBuffers(1, &buffer);

    // bindings of bound buffers are reset to 0
    size_t const targetIndex = getIndexForBufferTarget(target);
    auto& genericBinding = state.buffers.genericBinding[targetIndex];
    if (genericBinding == buffer) {
        genericBinding = 0;
    }

    if (target == GL_UNIFORM_BUFFER || target == GL_TRANSFORM_FEEDBACK_BUFFER) {
        auto& indexedBinding = state.buffers.targets[targetIndex];
        for (auto& entry: indexedBinding.buffers) {
            if (entry.name == buffer) {
                entry.name = 0;
                entry.offset = 0;
                entry.size = 0;
            }
        }
    }
}

void OpenGLContext::deleteVertexArray(GLuint vao) noexcept {
    if (vao) {
        glDeleteVertexArrays(1, &vao);
        // if the destroyed VAO is bound, clear the binding.
        if (state.vao.p->vao[m_ContextIndex] == vao) {
            bindVertexArray(nullptr);
        }
    }
}

void OpenGLContext::destroyWithContext(size_t index, std::function<void(OpenGLContext&)> const& closure) noexcept
{
}

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
GLuint OpenGLContext::getSamplerSlow(SamplerParams params) const noexcept {
    ASSERT(mSamplerMap.find(params) == mSamplerMap.end());

    GLuint s;
    glGenSamplers(1, &s);
    glSamplerParameteri(s, GL_TEXTURE_MIN_FILTER,   (GLint)RHI_Internal::GetGLFilter(params.filterMin));
    glSamplerParameteri(s, GL_TEXTURE_MAG_FILTER,   (GLint)RHI_Internal::GetGLFilter(params.filterMag));
    glSamplerParameteri(s, GL_TEXTURE_WRAP_S,       (GLint)RHI_Internal::GetGLWrapMode(params.wrapS));
    glSamplerParameteri(s, GL_TEXTURE_WRAP_T,       (GLint)RHI_Internal::GetGLWrapMode(params.wrapT));
    glSamplerParameteri(s, GL_TEXTURE_WRAP_R,       (GLint)RHI_Internal::GetGLWrapMode(params.wrapR));
    glSamplerParameteri(s, GL_TEXTURE_COMPARE_MODE, (GLint)RHI_Internal::GetGLCompareMode(params.compareMode));
    glSamplerParameteri(s, GL_TEXTURE_COMPARE_FUNC, (GLint)RHI_Internal::GetGLCompareFunc(params.compareFunc));

#if defined(GL_EXT_texture_filter_anisotropic)
    if (ext.EXT_texture_filter_anisotropic &&
        !bugs.texture_filter_anisotropic_broken_on_sampler) {
        GLfloat const anisotropy = float(1u << params.anisotropyLog2);
        glSamplerParameterf(s, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                std::min(gets.max_anisotropy, anisotropy));
    }
#endif
    // CHECK_GL_ERROR(utils::slog.e)
    mSamplerMap[params] = s;
    return s;
}
#endif

void OpenGLContext::resetState() noexcept {
    // Force GL state to match the Filament state

    // increase the state version so other parts of the state know to reset
    state.age++;

    if (state.major > 2) {
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, state.draw_fbo);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, state.read_fbo);
#endif
    } else {
        ASSERT(state.read_fbo == state.draw_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, state.draw_fbo);
        state.read_fbo = state.draw_fbo;
    }


    // state.program
    glUseProgram(state.program.use);

    // state.vao
    state.vao.p = nullptr;
    bindVertexArray(nullptr);

    // state.raster
    glFrontFace(state.raster.frontFace);
    glCullFace(state.raster.cullFace);
    glBlendEquationSeparate(state.raster.blendEquationRGB, state.raster.blendEquationA);
    glBlendFuncSeparate(
        state.raster.blendFunctionSrcRGB, 
        state.raster.blendFunctionDstRGB,
        state.raster.blendFunctionSrcA,
        state.raster.blendFunctionDstA
    );
    glColorMask(
        state.raster.colorMask, 
        state.raster.colorMask, 
        state.raster.colorMask, 
        state.raster.colorMask
    );
    glDepthMask(state.raster.depthMask);
    glDepthFunc(state.raster.depthFunc);
    
    // state.stencil
    glStencilFuncSeparate(
        GL_FRONT, 
        state.stencil.front.func.func, 
        state.stencil.front.func.ref, 
        state.stencil.front.func.mask
    );
    glStencilFuncSeparate(
        GL_BACK, 
        state.stencil.back.func.func, 
        state.stencil.back.func.ref, 
        state.stencil.back.func.mask
    );
    glStencilOpSeparate(
        GL_FRONT, 
        state.stencil.front.op.sfail,
        state.stencil.front.op.dpfail,
        state.stencil.front.op.dppass
    );
    glStencilOpSeparate(
        GL_BACK, 
        state.stencil.back.op.sfail,
        state.stencil.back.op.dpfail,
        state.stencil.back.op.dppass
    );
    glStencilMaskSeparate(GL_FRONT, state.stencil.front.stencilMask);
    glStencilMaskSeparate(GL_BACK, state.stencil.back.stencilMask);

    // state.polygonOffset
    glPolygonOffset(state.polygonOffset.factor, state.polygonOffset.units);

    // state.enables
    setDefaultState();

    // state.buffers
    // Reset state.buffers to its default state to avoid the complexity and error-prone
    // nature of resetting the GL state to its existing state
    state.buffers = {};

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    if (state.major > 2) {
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        for (auto const target: {
                GL_UNIFORM_BUFFER,
                GL_TRANSFORM_FEEDBACK_BUFFER,
#if defined(BACKEND_OPENGL_LEVEL_GLES31)
                GL_SHADER_STORAGE_BUFFER,
#endif
                GL_PIXEL_PACK_BUFFER,
                GL_PIXEL_UNPACK_BUFFER,
        }) {
            glBindBuffer(target, 0);
        }

        for (size_t bufferIndex = 0; bufferIndex < MAX_BUFFER_BINDINGS; ++bufferIndex) {
            if (bufferIndex < (size_t)gets.max_uniform_buffer_bindings) {
                glBindBufferBase(GL_UNIFORM_BUFFER, bufferIndex, 0);
            }

            if (bufferIndex < (size_t)gets.max_transform_feedback_separate_attribs) {
                glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, bufferIndex, 0);
            }
        }
#endif
    }

    // state.textures
    // Reset state.textures to its default state to avoid the complexity and error-prone
    // nature of resetting the GL state to its existing state
    state.textures = {};
    const std::pair<GLuint, bool> textureTargets[] = {
            { GL_TEXTURE_2D,                true },
            { GL_TEXTURE_2D_ARRAY,          true },
            { GL_TEXTURE_CUBE_MAP,          true },
            { GL_TEXTURE_3D,                true },
#if defined(BACKEND_OPENGL_LEVEL_GLES31)
            { GL_TEXTURE_2D_MULTISAMPLE,    true },
#endif
#if !defined(__EMSCRIPTEN__)
#if defined(GL_OES_EGL_image_external)
            { GL_TEXTURE_EXTERNAL_OES,      ext.OES_EGL_image_external_essl3 },
#endif
#if defined(BACKEND_OPENGL_VERSION_GL) || defined(GL_EXT_texture_cube_map_array)
            { GL_TEXTURE_CUBE_MAP_ARRAY,    ext.EXT_texture_cube_map_array },
#endif
#endif
    };
    for (GLint unit = 0; unit < gets.max_combined_texture_image_units; ++unit) {
        glActiveTexture(GL_TEXTURE0 + unit);
        if (state.major > 2) {
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
            glBindSampler(unit, 0);
#endif
        }
        for (auto [target, available] : textureTargets) {
            if (available) {
                glBindTexture(target, 0);
            }
        }
    }
    glActiveTexture(GL_TEXTURE0 + state.textures.active);

    // state.unpack
    glPixelStorei(GL_UNPACK_ALIGNMENT, state.unpack.alignment);
    if (state.major > 2) {
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        glPixelStorei(GL_UNPACK_ROW_LENGTH, state.unpack.row_length);
#endif
    }


    // state.pack
    glPixelStorei(GL_PACK_ALIGNMENT, state.pack.alignment);
    if (state.major > 2) {
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        glPixelStorei(GL_PACK_ROW_LENGTH, 0); // we rely on GL_PACK_ROW_LENGTH being zero
#endif
    }

    // state.window
    glScissor(
        state.window.scissor.x, 
        state.window.scissor.y, 
        state.window.scissor.z, 
        state.window.scissor.w
    );
    glViewport(
        state.window.viewport.x,
        state.window.viewport.y,
        state.window.viewport.z,
        state.window.viewport.w
    );
    glDepthRangef(state.window.depthRange.x, state.window.depthRange.y);
}
    
    void OpenGLContext::setDefaultState() const noexcept {
        // We need to make sure our internal state matches the GL state when we start.
        // (some of these calls may be unneeded as they might be the gl defaults)
        std::pair<GLenum, uint32_t> const caps[] = {
            {GL_BLEND, 1},
            {GL_CULL_FACE, 2},
            {GL_SCISSOR_TEST, 3},
            {GL_DEPTH_TEST, 4},
            {GL_STENCIL_TEST, 5},
            {GL_DITHER, 6},
            {GL_SAMPLE_ALPHA_TO_COVERAGE, 7},
            {GL_SAMPLE_COVERAGE, 8},
            {GL_POLYGON_OFFSET_FILL, 9},  
        };

        for (auto [cap, index] : caps) {
            if (state.enables.caps & (1 << index)) {
                glEnable(cap);
            } else {
                glDisable(cap);
            }
        }
    
    }

constexpr size_t OpenGLContext::getIndexForBufferTarget(GLenum target) noexcept {
    size_t index = 0;
    switch (target) {
        // The indexed buffers MUST be first in this list (those usable with bindBufferRange)
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        case GL_UNIFORM_BUFFER:             index = 0; break;
        case GL_TRANSFORM_FEEDBACK_BUFFER:  index = 1; break;
        case GL_SHADER_STORAGE_BUFFER:      index = 2; break;
#endif
        case GL_ARRAY_BUFFER:               index = 3; break;
        case GL_ELEMENT_ARRAY_BUFFER:       index = 4; break;
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        case GL_PIXEL_PACK_BUFFER:          index = 5; break;
        case GL_PIXEL_UNPACK_BUFFER:        index = 6; break;
#endif
        default: break;
    }
    ASSERT(index < sizeof(state.buffers.genericBinding)/sizeof(state.buffers.genericBinding[0])); // NOLINT(misc-redundant-expression)
    return index;
}
}
