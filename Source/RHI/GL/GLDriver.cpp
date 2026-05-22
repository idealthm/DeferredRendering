#include "GLDriver.h"

#include <algorithm>
#include <glm/gtc/type_ptr.inl>

#include "GLHelper.h"
#include "GLProgram.h"
#include "RHI/PipelineState.h"
#include "RHI/PixelBufferDescriptor.h"
#include "RHI/RHIDriver.h"
#include "RHI/GL/GLDescriptorSet.h"
#include "RHI/GL/GLDescriptorSetLayout.h"


namespace RHI
{
	GLDriver::GLDriver()
		: m_HandleAllocator(4U * 1024U * 1024U)
	{
	}

	void GLDriver::beginRenderPass(Handle<HwRenderTarget> h, RenderPassParams& params)
	{
		auto& gl = m_Context;

    mRenderPassTarget = h;
    mRenderPassParams = params;

    GLRenderTarget* rt = handle_cast<GLRenderTarget*>(h);

    // If we're rendering into the default render target (i.e. into the current SwapChain),
    // get the value of the output colorspace from there, otherwise it's always linear.

    const TargetBufferFlags clearFlags = params.flags.clear & rt->targets;
    TargetBufferFlags discardFlags = params.flags.discardStart & rt->targets;

    GLuint const fbo = gl.bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);

    // each render-pass starts with a disabled scissor
    gl.disable(GL_SCISSOR_TEST);

    AttachmentArray attachments; // NOLINT
    GLsizei const attachmentCount = getAttachments(attachments, discardFlags, !fbo);
    if (attachmentCount) {
        glInvalidateFramebuffer(GL_FRAMEBUFFER, attachmentCount, attachments.data());
    }
	{
        // It's important to clear the framebuffer before drawing, as it resets
        // the fb to a known state (resets fb compression and possibly other things).
        // So we use glClear instead of glInvalidateFramebuffer
       //  clearWithRasterPipe(discardFlags & ~clearFlags, { 0.0f }, 0.0f, 0);
    }

    if (rt->gl.fbo_read) {
        // we have a multi-sample RenderTarget with non multi-sample attachments (i.e. this is the
        // EXT_multisampled_render_to_texture emulation).
        // We would need to perform a "backward" resolve, i.e. load the resolved texture into the
        // tile, everything must appear as though the multi-sample buffer was lost.
        // However, Filament specifies that a non multi-sample attachment to a
        // multi-sample RenderTarget is always discarded. We do this because implementing
        // the load on Metal is not trivial, and it's not a feature we rely on at this time.
        discardFlags |= rt->gl.resolve;
    }

    if (any(clearFlags)) {
        clearWithRasterPipe(clearFlags,
                params.clearColor, (GLfloat)params.clearDepth, (GLint)params.clearStencil);
    }

    // we need to reset those after we call clearWithRasterPipe()
    mRenderPassColorWrite   = any(clearFlags & TargetBufferFlags::COLOR_ALL);
    mRenderPassDepthWrite   = any(clearFlags & TargetBufferFlags::DEPTH);
    mRenderPassStencilWrite = any(clearFlags & TargetBufferFlags::STENCIL);

    static_assert(sizeof(GLsizei) >= sizeof(uint32_t));
    gl.viewport(params.viewport.left, params.viewport.bottom,
            (GLsizei)std::min(uint32_t(std::numeric_limits<int32_t>::max()), params.viewport.width),
            (GLsizei)std::min(uint32_t(std::numeric_limits<int32_t>::max()), params.viewport.height));

    gl.depthRange(params.depthRange.near, params.depthRange.far);

#ifndef NDEBUG
    // clear the discarded (but not the cleared ones) buffers in debug builds
    clearWithRasterPipe(discardFlags & ~clearFlags,
            { 1, 0, 0, 1 }, 1.0, 0);
#endif
	}

	void GLDriver::endRenderPass()
	{
		
	}

	GLsizei GLDriver::getAttachments(AttachmentArray& attachments, TargetBufferFlags buffers, bool isDefaultFramebuffer) noexcept {
		GLsizei attachmentCount = 0;
		// the default framebuffer uses different constants!!!

		if (any(buffers & TargetBufferFlags::COLOR0)) {
			attachments[attachmentCount++] = isDefaultFramebuffer ? GL_COLOR : GL_COLOR_ATTACHMENT0;
		}
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
		if (any(buffers & TargetBufferFlags::COLOR1)) {
			ASSERT(!isDefaultFramebuffer);
			attachments[attachmentCount++] = GL_COLOR_ATTACHMENT1;
		}
		if (any(buffers & TargetBufferFlags::COLOR2)) {
			ASSERT(!isDefaultFramebuffer);
			attachments[attachmentCount++] = GL_COLOR_ATTACHMENT2;
		}
		if (any(buffers & TargetBufferFlags::COLOR3)) {
			ASSERT(!isDefaultFramebuffer);
			attachments[attachmentCount++] = GL_COLOR_ATTACHMENT3;
		}
		if (any(buffers & TargetBufferFlags::COLOR4)) {
			ASSERT(!isDefaultFramebuffer);
			attachments[attachmentCount++] = GL_COLOR_ATTACHMENT4;
		}
		if (any(buffers & TargetBufferFlags::COLOR5)) {
			ASSERT(!isDefaultFramebuffer);
			attachments[attachmentCount++] = GL_COLOR_ATTACHMENT5;
		}
		if (any(buffers & TargetBufferFlags::COLOR6)) {
			ASSERT(!isDefaultFramebuffer);
			attachments[attachmentCount++] = GL_COLOR_ATTACHMENT6;
		}
		if (any(buffers & TargetBufferFlags::COLOR7)) {
			ASSERT(!isDefaultFramebuffer);
			attachments[attachmentCount++] = GL_COLOR_ATTACHMENT7;
		}
#endif
		if (any(buffers & TargetBufferFlags::DEPTH)) {
			attachments[attachmentCount++] = isDefaultFramebuffer ? GL_DEPTH : GL_DEPTH_ATTACHMENT;
		}
		if (any(buffers & TargetBufferFlags::STENCIL)) {
			attachments[attachmentCount++] = isDefaultFramebuffer ? GL_STENCIL : GL_STENCIL_ATTACHMENT;
		}
		return attachmentCount;
	}

	void GLDriver::draw2(size_t offset, size_t count, uint32_t instanceCount)
	{
		ASSERT(mBoundRenderPrimitive);

		ASSERT(mBoundProgram);
		ASSERT(mValidProgram);

		// When the program changes, we might have to rebind all or some descriptors
		auto const invalidDescriptorSets =
				mInvalidDescriptorSetBindings | mInvalidDescriptorSetBindingOffsets;
		if (UTILS_UNLIKELY(invalidDescriptorSets.any())) {
			updateDescriptors(invalidDescriptorSets);
		}

		GLRenderPrimitive const* const rp = mBoundRenderPrimitive;
		glDrawElementsInstanced(GLenum(rp->type), (GLsizei)count,
				rp->gl.getIndicesType(),
				reinterpret_cast<const void*>(offset << rp->gl.indicesShift),
				(GLsizei)instanceCount);
	}

	void GLDriver::draw(PipelineState state, Handle<HwRenderPrimitive> rph, uint32_t indexOffset, uint32_t indexCount, uint32_t instanceCount)
	{
		GLRenderPrimitive* const rp = handle_cast<GLRenderPrimitive*>(rph);
		state.primitiveType = rp->type;
		state.vertexBufferInfo = rp->vbih;
		bindPipeline(state);
		bindRenderPrimitive(rph);
		draw2(indexOffset, indexCount, instanceCount);
	}

	void GLDriver::bindPipeline(PipelineState const& state)
	{
		setRasterState(state.rasterState);
		setStencilState(state.stencilState);
		// gl.polygonOffset(state.polygonOffset.slope, state.polygonOffset.constant);
		GLProgram* const p = handle_cast<GLProgram*>(state.program);
		mValidProgram = useProgram(p);
		// (*mCurrentPushConstants) = p->getPushConstants();
		// mCurrentSetLayout = state.pipelineLayout.setLayout;
	}

	void GLDriver::bindRenderPrimitive(Handle<HwRenderPrimitive> rph)
	{
		auto& gl = m_Context;

		GLRenderPrimitive* const rp = handle_cast<GLRenderPrimitive*>(rph);

		// Gracefully do nothing if the render primitive has not been set up.
		VertexBufferHandle vb = rp->gl.vertexBufferWithObjects;
		if (UTILS_UNLIKELY(!vb)) {
			mBoundRenderPrimitive = nullptr;
			return;
		}

		// If necessary, mutate the bindings in the VAO.
		gl.bindVertexArray(&rp->gl);
		GLVertexBuffer const* const glvb = handle_cast<GLVertexBuffer*>(vb);
		updateVertexArrayObject(rp, glvb);

		mBoundRenderPrimitive = rp;
	}

	void GLDriver::clearWithRasterPipe(TargetBufferFlags clearFlags, glm::vec4 const& linearColor, GLfloat depth,  GLint stencil) noexcept
	{
		if (any(clearFlags & TargetBufferFlags::COLOR_ALL)) {
			m_Context.colorMask(GL_TRUE);
		}
		if (any(clearFlags & TargetBufferFlags::DEPTH)) {
			m_Context.depthMask(GL_TRUE);
		}
		if (any(clearFlags & TargetBufferFlags::STENCIL)) {
			m_Context.stencilMaskSeparate(0xFF, m_Context.state.stencil.back.stencilMask);
		}

		if (any(clearFlags & TargetBufferFlags::COLOR0)) {
			glClearBufferfv(GL_COLOR, 0, glm::value_ptr(linearColor));
		}
		if (any(clearFlags & TargetBufferFlags::COLOR1)) {
			glClearBufferfv(GL_COLOR, 1, glm::value_ptr(linearColor));
		}
		if (any(clearFlags & TargetBufferFlags::COLOR2)) {
			glClearBufferfv(GL_COLOR, 2, glm::value_ptr(linearColor));
		}
		if (any(clearFlags & TargetBufferFlags::COLOR3)) {
			glClearBufferfv(GL_COLOR, 3, glm::value_ptr(linearColor));
		}
		if (any(clearFlags & TargetBufferFlags::COLOR4)) {
			glClearBufferfv(GL_COLOR, 4, glm::value_ptr(linearColor));
		}
		if (any(clearFlags & TargetBufferFlags::COLOR5)) {
			glClearBufferfv(GL_COLOR, 5, glm::value_ptr(linearColor));
		}
		if (any(clearFlags & TargetBufferFlags::COLOR6)) {
			glClearBufferfv(GL_COLOR, 6, glm::value_ptr(linearColor));
		}
		if (any(clearFlags & TargetBufferFlags::COLOR7)) {
			glClearBufferfv(GL_COLOR, 7, glm::value_ptr(linearColor));
		}

		if ((clearFlags & TargetBufferFlags::DEPTH_AND_STENCIL) == TargetBufferFlags::DEPTH_AND_STENCIL) {
			glClearBufferfi(GL_DEPTH_STENCIL, 0, depth, stencil);
		} else {
			if (any(clearFlags & TargetBufferFlags::DEPTH)) {
				glClearBufferfv(GL_DEPTH, 0, &depth);
			}
			if (any(clearFlags & TargetBufferFlags::STENCIL)) {
				glClearBufferiv(GL_STENCIL, 0, &stencil);
			}
		}
	}

	void GLDriver::setRasterState(RasterState rs) noexcept
	{
		auto& gl = m_Context;

		mRenderPassColorWrite |= rs.colorWrite;
		mRenderPassDepthWrite |= rs.depthWrite;

		// culling state
		if (rs.culling == CullingMode::NONE) {
			gl.disable(GL_CULL_FACE);
		} else {
			gl.enable(GL_CULL_FACE);
			gl.cullFace(RHI_Internal::GetGLCullingMode(rs.culling));
		}

		gl.frontFace(rs.inverseFrontFaces ? GL_CW : GL_CCW);

		// blending state
		if (!rs.hasBlending()) {
			gl.disable(GL_BLEND);
		} else {
			gl.enable(GL_BLEND);
			gl.blendEquation(
					RHI_Internal::GetGLBlendEquationMode(rs.blendEquationRGB),
					RHI_Internal::GetGLBlendEquationMode(rs.blendEquationAlpha));

			gl.blendFunction(
					RHI_Internal::GetGLBlendFunctionMode(rs.blendFunctionSrcRGB),
					RHI_Internal::GetGLBlendFunctionMode(rs.blendFunctionSrcAlpha),
					RHI_Internal::GetGLBlendFunctionMode(rs.blendFunctionDstRGB),
					RHI_Internal::GetGLBlendFunctionMode(rs.blendFunctionDstAlpha));
		}

		// depth test
		if (rs.depthFunc == SamplerCompareFunc::Always && !rs.depthWrite) {
			gl.disable(GL_DEPTH_TEST);
		} else {
			gl.enable(GL_DEPTH_TEST);
			gl.depthFunc(RHI_Internal::GetGLCompareFunc(rs.depthFunc));
			gl.depthMask(GLboolean(rs.depthWrite));
		}

		// write masks
		gl.colorMask(GLboolean(rs.colorWrite));

		// AA
		if (rs.alphaToCoverage) {
			gl.enable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		} else {
			gl.disable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		}

		if (rs.depthClamp) {
			gl.enable(GL_DEPTH_CLAMP);
		} else {
			gl.disable(GL_DEPTH_CLAMP);
		}
	}

	void GLDriver::setStencilState(StencilState ss) noexcept
	{
		auto& gl = m_Context;

		mRenderPassStencilWrite |= ss.stencilWrite;

		// stencil test / operation
		// GL_STENCIL_TEST must be enabled if we're testing OR writing to the stencil buffer.
		if (ss.front.stencilFunc == SamplerCompareFunc::Always &&
				ss.back.stencilFunc == SamplerCompareFunc::Always &&
				ss.front.stencilOpDepthFail == StencilOperation::KEEP &&
				ss.back.stencilOpDepthFail == StencilOperation::KEEP &&
				ss.front.stencilOpStencilFail == StencilOperation::KEEP &&
				ss.back.stencilOpStencilFail == StencilOperation::KEEP &&
				ss.front.stencilOpDepthStencilPass == StencilOperation::KEEP &&
				ss.back.stencilOpDepthStencilPass == StencilOperation::KEEP) {
			// that's equivalent to having the stencil test disabled
			gl.disable(GL_STENCIL_TEST);
				} else {
					gl.enable(GL_STENCIL_TEST);
				}

		// glStencilFuncSeparate() also sets the reference value, which may be used depending
		// on the stencilOp, so we always need to call glStencilFuncSeparate().
		gl.stencilFuncSeparate(
			RHI_Internal::GetGLCompareFunc(ss.front.stencilFunc), ss.front.ref, ss.front.readMask,
			RHI_Internal::GetGLCompareFunc(ss.back.stencilFunc), ss.back.ref, ss.back.readMask);

		if (!ss.stencilWrite) {
			gl.stencilMaskSeparate(0x00, 0x00);
		} else {
			// Stencil ops are only relevant when stencil write is enabled
			gl.stencilOpSeparate(
				RHI_Internal::GetGLStencilOperation(ss.front.stencilOpStencilFail),
				RHI_Internal::GetGLStencilOperation(ss.front.stencilOpDepthFail),
				RHI_Internal::GetGLStencilOperation(ss.front.stencilOpDepthStencilPass),
				RHI_Internal::GetGLStencilOperation(ss.back.stencilOpStencilFail),
				RHI_Internal::GetGLStencilOperation(ss.back.stencilOpDepthFail),
				RHI_Internal::GetGLStencilOperation(ss.back.stencilOpDepthStencilPass));
			gl.stencilMaskSeparate(ss.front.writeMask, ss.back.writeMask);
		}
	}

	Handle<HwProgram> GLDriver::CreateProgram(Program&& program)
	{
		Handle<HwProgram> h = InitHandle<GLProgram>();
		CreateProgram(h, std::move(program));
		return h;
	}
	Handle<HwTexture> GLDriver::CreateTexture(SamplerType target, uint8_t levels, Format format, uint8_t samples, uint32_t width, uint32_t height, uint32_t depth, TextureUsage usage)
	{
		Handle<HwTexture> h = InitHandle<GLTexture>();
		CreateTexture(h, target, levels, format, samples, width, height, depth, usage);
		return h;
	}
	Handle<HwBufferObject> GLDriver::CreateBufferObject(size_t size, BufferObjectBinding target, BufferUsage usage)
	{
		Handle<HwBufferObject> h = InitHandle<GLBufferObject>();
		CreateBufferObject(h, size, target, usage);
		return h;
	}
	Handle<HwVertexBufferInfo> GLDriver::CreateVertexBufferInfo(size_t bufferCount, size_t attributeCount, AttributeArray attribute)
	{
		Handle<HwVertexBufferInfo> h = InitHandle<GLVertexBufferInfo>();
		CreateVertexBufferInfo(h, bufferCount, attributeCount, attribute);
		return h;
	}
	Handle<HwVertexBuffer> GLDriver::CreateVertexBuffer(size_t vertexCount, Handle<HwVertexBufferInfo> info)
	{
		Handle<HwVertexBuffer> h = InitHandle<GLVertexBuffer>();
		CreateVertexBuffer(h, vertexCount, info);
		return h;
	}
	Handle<HwIndexBuffer> GLDriver::CreateIndexBuffer(ElementType type, size_t indexCount, BufferUsage usage)
	{
		Handle<HwIndexBuffer> h = InitHandle<GLIndexBuffer>();
		CreateIndexBuffer(h, type, indexCount, usage);
		return h;
	}
	Handle<HwRenderPrimitive> GLDriver::CreateRenderPrimitive(Handle<HwVertexBuffer> vbh, Handle<HwIndexBuffer> ibh, PrimitiveType type)
	{
		Handle<HwRenderPrimitive> h = InitHandle<GLRenderPrimitive>();
		CreateRenderPrimitive(h, vbh, ibh, type);
		return h;
	}
	Handle<HwRenderTarget> GLDriver::CreateRenderTarget(TargetBufferFlags targets, uint32_t width, uint32_t height,
		uint8_t samplers, uint8_t layerCount, MRT color, TargetBufferInfo depth, TargetBufferInfo stencil)
	{
		Handle<HwRenderTarget> h = InitHandle<GLRenderTarget>();
		CreateRenderTarget(h, targets, width, height, samplers, layerCount, color, depth, stencil);
		return h;
	}
	Handle<HwDescriptorSet> GLDriver::CreateDescriptorSet(Handle<HwDescriptorSetLayout> dslh)
	{
		Handle<HwDescriptorSet> h = InitHandle<GLDescriptorSet>();
		CreateDescriptorSet(h, dslh);
		return h;
	}
	Handle<HwDescriptorSetLayout> GLDriver::CreateDescriptorSetLayout(DescriptorSetLayout&& info)
	{
		Handle<HwDescriptorSetLayout> h = InitHandle<GLDescriptorSetLayout>();
		CreateDescriptorSetLayout(h, std::move(info));
		return h;
	}

	void GLDriver::CreateProgram(Handle<HwProgram> h, Program&& program)
	{
		construct<GLProgram>(h, this, std::forward<Program&&>(program));
	}

	void GLDriver::CreateTexture(Handle<HwTexture> h, SamplerType target, uint8_t levels, Format format, uint8_t samples, uint32_t width, uint32_t height, uint32_t depth, TextureUsage usage)
	{
		GLenum internalFormat = RHI_Internal::GetGLInternalFormat(format);

	    auto& gl = m_Context;
	    samples = std::clamp(samples, uint8_t(1u), uint8_t(gl.gets.max_samples));
	    GLTexture* t = construct<GLTexture>(h, target, levels, samples, width, height, depth, format, usage);

		if (any(usage & TextureUsage::SAMPLEABLE))
		{
			glGenTextures(1, &t->gl.id);

			t->gl.internalFormat = internalFormat;

			t->gl.target = RHI_Internal::GetGLTextureTarget(target);

			textureStorage(t, width, height, depth);
		}
		else
		{
			t->gl.internalFormat = internalFormat;
			t->gl.target = GL_RENDERBUFFER;
			glGenRenderbuffers(1, &t->gl.id);
			renderBufferStorage(t->gl.id, internalFormat, width, height, samples);
		}
    }

	void GLDriver::CreateBufferObject(Handle<HwBufferObject> h, size_t count, BufferObjectBinding target, BufferUsage usage)
	{
		auto& gl = m_Context;
		if (target == BufferObjectBinding::VERTEX)
			gl.bindVertexArray(nullptr);

		GLBufferObject* bo = construct<GLBufferObject>(h, count, target, usage);
		bo->gl.binding = RHI_Internal::GetGLBufferBinding(target);
		glGenBuffers(1, &bo->gl.id);
		gl.bindBuffer(bo->gl.binding, bo->gl.id);
		glBufferData(bo->gl.binding, bo->byteCount, nullptr, RHI_Internal::GetGLBufferUsage(usage));
	}

	void GLDriver::CreateVertexBufferInfo(Handle<HwVertexBufferInfo> h, size_t bufferCount, size_t attributeCount, AttributeArray attributes)
	{
		construct<GLVertexBufferInfo>(h, bufferCount, attributeCount, attributes);
	}

	void GLDriver::CreateVertexBuffer(Handle<HwVertexBuffer> h, size_t vertexCount, Handle<HwVertexBufferInfo> info)
	{
		construct<GLVertexBuffer>(h, vertexCount, info);
	}

	void GLDriver::CreateIndexBuffer(Handle<HwIndexBuffer> h, ElementType type, size_t indexCount, BufferUsage usage)
	{
		auto& gl = m_Context;
		size_t eleSize = RHI_Internal::GetGLElementSize(type);
		GLIndexBuffer* ib = construct<GLIndexBuffer>(h, indexCount, eleSize);
		glGenBuffers(1, &ib->gl.buffer);
		const size_t size = indexCount * eleSize;
		gl.bindVertexArray(nullptr);
		gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->gl.buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, nullptr, RHI_Internal::GetGLBufferUsage(usage));
	}

	void GLDriver::CreateRenderPrimitive(Handle<HwRenderPrimitive> h, Handle<HwVertexBuffer> vbh, Handle<HwIndexBuffer> ibh, PrimitiveType type)
	{
		auto& gl = m_Context;

		GLIndexBuffer const* const ib = handle_cast<const GLIndexBuffer*>(ibh);
		ASSERT(ib->elementSize == 2 || ib->elementSize == 4);

		GLVertexBuffer* vb = handle_cast<GLVertexBuffer*>(vbh);
		GLRenderPrimitive* rp =  handle_cast<GLRenderPrimitive*>(h); // needn't  construct
		rp->gl.indicesShift = (ib->elementSize == 4u) ? 2 : 1;
		rp->gl.indicesType = (ib->elementSize == 4u) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
		rp->gl.vertexBufferWithObjects = vbh;
		rp->type = type;
		rp->vbih = vb->vbih;

		glGenVertexArrays(1, &rp->gl.vao[gl.m_ContextIndex]);

		rp->gl.nameVersion = gl.state.age;

		gl.bindVertexArray(&rp->gl);

		gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->gl.buffer);
	}

	void GLDriver::CreateRenderTarget(Handle<HwRenderTarget> h, TargetBufferFlags targets, uint32_t width, uint32_t height, uint8_t samples, uint8_t layerCount, MRT color, TargetBufferInfo depth, TargetBufferInfo stencil)
	{
		GLRenderTarget* rt = construct<GLRenderTarget>(h, width, height);
		glGenFramebuffers(1, &rt->gl.fbo);

		samples = std::clamp(samples, uint8_t(1u), uint8_t(m_Context.gets.max_samples));

		rt->gl.samples = samples;
		rt->targets = targets;

		glm::uvec2 tmin (std::numeric_limits<uint32_t>::max());
		glm::uvec2 tmax (0);

		auto checkDimensions = [&tmin, &tmax](GLTexture* t, uint8_t level) {
			const auto twidth = std::max(1u, t->width >> level);
			const auto theight = std::max(1u, t->height >> level);
			tmin = { std::min(tmin.x, twidth), std::min(tmin.y, theight) };
			tmax = { std::max(tmax.x, twidth), std::max(tmax.y, theight) };
		};

		if (any(targets & TargetBufferFlags::COLOR_ALL)) {
			GLenum bufs[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = { GL_NONE };
			const size_t maxDrawBuffers = getMaxDrawBuffers();
			for (size_t i = 0; i < maxDrawBuffers; i++) {
				if (any(targets & getTargetBufferFlagsAt(i))) {
					ASSERT(color[i].handle);
					rt->gl.color[i] = handle_cast<GLTexture*>(color[i].handle);
					framebufferTexture(color[i], rt, GL_COLOR_ATTACHMENT0 + i, layerCount);
					bufs[i] = GL_COLOR_ATTACHMENT0 + i;
					checkDimensions(rt->gl.color[i], color[i].level);
				}
			}
			glDrawBuffers((GLsizei)maxDrawBuffers, bufs);
		}

		bool specialCased = false;

		if ((targets & TargetBufferFlags::DEPTH_AND_STENCIL) == TargetBufferFlags::DEPTH_AND_STENCIL) {
			ASSERT(depth.handle);
			// either we supplied only the depth handle or both depth/stencil are identical and not null
			if (depth.handle && (stencil.handle == depth.handle || !stencil.handle)) {
				rt->gl.depth = handle_cast<GLTexture*>(depth.handle);
				framebufferTexture(depth, rt, GL_DEPTH_STENCIL_ATTACHMENT, layerCount);
				specialCased = true;
				checkDimensions(rt->gl.depth, depth.level);
			}
		}

		if (!specialCased)
		{
			if (any(targets & TargetBufferFlags::DEPTH)) {
				ASSERT(depth.handle);
				rt->gl.depth = handle_cast<GLTexture*>(depth.handle);
				framebufferTexture(depth, rt, GL_DEPTH_ATTACHMENT, layerCount);
				checkDimensions(rt->gl.depth, depth.level);
			}
			if (any(targets & TargetBufferFlags::STENCIL)) {
				ASSERT(stencil.handle);
				rt->gl.stencil = handle_cast<GLTexture*>(stencil.handle);
				framebufferTexture(stencil, rt, GL_STENCIL_ATTACHMENT, layerCount);
				checkDimensions(rt->gl.stencil, stencil.level);
			}
		}
	}
	void GLDriver::CreateDescriptorSet(Handle<HwDescriptorSet> h, Handle<HwDescriptorSetLayout> dslh)
	{
		GLDescriptorSetLayout const* dsl = handle_cast<GLDescriptorSetLayout*>(dslh);
		construct<GLDescriptorSet>(h, m_Context, dslh, dsl);
	}

	void GLDriver::CreateDescriptorSetLayout(Handle<HwDescriptorSetLayout> h, DescriptorSetLayout&& info)
	{
		construct<GLDescriptorSetLayout>(h, std::move(info));
	}

	void GLDriver::DestroyProgram(Handle<HwProgram> h)
	{
		if (h)
		{
			GLProgram* p = handle_cast<GLProgram*>(h);
			destruct(h, p);
		}
	}

	void GLDriver::DestroyTexture(Handle<HwTexture> h)
	{
		if (h) {
			auto& gl = m_Context;
			GLTexture* t = handle_cast<GLTexture*>(h);

			if ((!t->gl.imported)) {
				if (any(t->usage & TextureUsage::SAMPLEABLE)) {
					// drop a reference
					uint16_t count = 0;
					if (UTILS_UNLIKELY(t->ref)) {
						// the common case is that we don't have a ref handle
						GLTextureRef* const ref = handle_cast<GLTextureRef*>(t->ref);
						count = --(ref->count);
						if (count == 0) {
							destruct(t->ref, ref);
						}
					}
					if (count == 0) {
						// if this was the last reference, we destroy the refcount as well as
						// the GL texture name itself.
						gl.unbindTexture(t->gl.target, t->gl.id);
						glDeleteTextures(1, &t->gl.id);
					} else {
						// The Handle<HwTexture> is always destroyed. For extra precaution we also
						// check that the GLTexture has a trivial destructor.
						static_assert(std::is_trivially_destructible_v<GLTexture>);
					}
				} else {
					ASSERT(t->gl.target == GL_RENDERBUFFER);
					glDeleteRenderbuffers(1, &t->gl.id);
				}
				if (t->gl.sidecarRenderBufferMS) {
					glDeleteRenderbuffers(1, &t->gl.sidecarRenderBufferMS);
				}
			} else {
				gl.unbindTexture(t->gl.target, t->gl.id);
			}
			destruct(h, t);
		}
	}

	void GLDriver::DestroyBuffer(Handle<HwBufferObject> h)
	{
		if (h) {
			auto& gl = m_Context;
			GLBufferObject const* bo = handle_cast<const GLBufferObject*>(h);
			gl.deleteBuffer(bo->gl.id, bo->gl.binding);
			destruct(h, bo);
		}
	}

	void GLDriver::DestroyVertexBufferInfo(Handle<HwVertexBufferInfo> h)
	{
		if (h) {
			GLVertexBufferInfo const* vbi = handle_cast<const GLVertexBufferInfo*>(h);
			destruct(h, vbi);
		}
	}

	void GLDriver::DestroyVertexBuffer(Handle<HwVertexBuffer> h)
	{
		if (h) {
			GLVertexBuffer const* vb = handle_cast<const GLVertexBuffer*>(h);
			destruct(h, vb);
		}
	}

	void GLDriver::DestroyIndexBuffer(Handle<HwIndexBuffer> h)
	{
		if (h) {
			auto& gl = m_Context;
			GLIndexBuffer const* ib = handle_cast<const GLIndexBuffer*>(h);
			gl.deleteBuffer(ib->gl.buffer, GL_ELEMENT_ARRAY_BUFFER);
			destruct(h, ib);
		}
	}

	void GLDriver::DestroyRenderPrimitive(Handle<HwRenderPrimitive> h)
	{
		if (h) {
			auto& gl = m_Context;
			GLRenderPrimitive const* rp = handle_cast<const GLRenderPrimitive*>(h);
			gl.deleteVertexArray(rp->gl.vao[gl.m_ContextIndex]);

			// If we have a name in the "other" context, we need to schedule the destroy for
			// later, because it can't be done here. VAOs are "container objects" and are not
			// shared between contexts.
			size_t const otherContextIndex = 1 - gl.m_ContextIndex;
			GLuint const nameInOtherContext = rp->gl.vao[otherContextIndex];
			if (UTILS_UNLIKELY(nameInOtherContext)) {
				gl.destroyWithContext(otherContextIndex,
						[name = nameInOtherContext](OpenGLContext& gl) {
					gl.deleteVertexArray(name);
				});
			}

			destruct(h, rp);
		}
	}

	void GLDriver::DestroyRenderTarget(Handle<HwRenderTarget> h)
	{
		if (h) {
			auto& gl = m_Context;
			GLRenderTarget* rt = handle_cast<GLRenderTarget*>(h);
			if (rt->gl.fbo) {
				// first unbind this framebuffer if needed
				gl.unbindFramebuffer(GL_FRAMEBUFFER);
			}
			if (rt->gl.fbo_read) {
				// first unbind this framebuffer if needed
				gl.unbindFramebuffer(GL_FRAMEBUFFER);
			}

/*#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
			if (UTILS_UNLIKELY(gl.bugs.delay_fbo_destruction)) {
				if (rt->gl.fbo) {
					whenFrameComplete([fbo = rt->gl.fbo]() {
						glDeleteFramebuffers(1, &fbo);
					});
				}
				if (rt->gl.fbo_read) {
					whenFrameComplete([fbo_read = rt->gl.fbo_read]() {
						glDeleteFramebuffers(1, &fbo_read);
					});
				}
			} else
#endif*/
			{
				if (rt->gl.fbo) {
					glDeleteFramebuffers(1, &rt->gl.fbo);
				}
				if (rt->gl.fbo_read) {
					glDeleteFramebuffers(1, &rt->gl.fbo_read);
				}
			}
			destruct(h, rt);
		}
	}

	void GLDriver::DestroyDescriptorSet(Handle<HwDescriptorSet> h)
	{
		if (h) {
			// unbind the descriptor-set, to avoid use-after-free
			for (auto& bound : mBoundDescriptorSets) {
				if (bound.dsh == h) {
					bound = {};
				}
			}
			GLDescriptorSet* ds = handle_cast<GLDescriptorSet*>(h);
			destruct(h, ds);
		}
	}

	void GLDriver::DestroyDescriptorSetLayout(Handle<HwDescriptorSetLayout> h)
	{
		if (h) {
			GLDescriptorSetLayout* dsl = handle_cast<GLDescriptorSetLayout*>(h);
			destruct(h, dsl);
		}
	}

	void GLDriver::updateBufferObject(Handle<HwBufferObject> boh, BufferDescriptor&& data, uint32_t byteOffset)
	{
		auto& gl = m_Context;
		GLBufferObject* bo = handle_cast<GLBufferObject*>(boh);
		gl.bindBuffer(bo->gl.binding, bo->gl.id);
		if (byteOffset == 0 && data.size == bo->byteCount) {
			glBufferData(bo->gl.binding, data.size, data.buffer, RHI_Internal::GetGLBufferUsage(bo->usage));
		} else {
			glBufferSubData(bo->gl.binding, byteOffset, data.size, data.buffer);
		}
	}

	void GLDriver::setVertexBufferObject(Handle<HwVertexBuffer> vbh, uint8_t bufferSlot, Handle<HwBufferObject> boh)
	{
		auto& gl = m_Context;
		GLVertexBuffer* vb = handle_cast<GLVertexBuffer*>(vbh);
		GLBufferObject const* bo = handle_cast<const GLBufferObject*>(boh);

		if (vb->gl.buffers[bufferSlot] != bo->gl.id) {
			vb->gl.buffers[bufferSlot] = bo->gl.id;
			vb->bufferObjectsVersion++;
		}
	}

	void GLDriver::setIndexBufferObject(Handle<HwIndexBuffer> ibh, Handle<HwBufferObject> boh)
	{
		auto& gl = m_Context;
		GLIndexBuffer* ib = handle_cast<GLIndexBuffer*>(ibh);
		GLBufferObject const* bo = handle_cast<const GLBufferObject*>(boh);

		ib->gl.buffer = bo->gl.id;
	}

	uint32_t GLDriver::getMaxDrawBuffers() const
	{
		return std::min(MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT, uint8_t(m_Context.gets.max_draw_buffers));
	}

	bool GLDriver::CompileShader(const Program::ShaderSource& src, uint32_t& program) const
	{
		auto& gl = m_Context;

		GLuint shaderProgram = glCreateProgram();

		std::vector<GLuint> shaders;

		auto compileShader = [&](GLenum type, const std::vector<uint8_t>& source) -> GLuint {
			if (source.empty()) return 0;

			GLuint shader = glCreateShader(type);
			
			const char* sourceStr = reinterpret_cast<const char*>(source.data());
			GLint length = static_cast<GLint>(source.size());
			
			glShaderSource(shader, 1, &sourceStr, &length);
			glCompileShader(shader);

			GLint success;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success) {
				char infoLog[512];
				glGetShaderInfoLog(shader, 512, nullptr, infoLog);
				ASSERT(false && "Shader compilation failed");
				glDeleteShader(shader);
				return 0;
			}

			glAttachShader(shaderProgram, shader);
			shaders.push_back(shader);
			return shader;
		};

		compileShader(GL_VERTEX_SHADER, src[0]);
		compileShader(GL_FRAGMENT_SHADER, src[1]);
		// compileShader(GL_COMPUTE_SHADER, src[2]);

		glLinkProgram(shaderProgram);

		GLint success;
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
			ASSERT(false && "Program linking failed");
			
			for (GLuint shader : shaders) {
				glDeleteShader(shader);
			}
			glDeleteProgram(shaderProgram);
			return false;
		}

		for (GLuint shader : shaders) {
			glDeleteShader(shader);
		}

		program = shaderProgram;
		return true;
	}

	void GLDriver::generateMipmap(Handle<HwTexture> handle)
	{
		auto& gl = m_Context;
		GLTexture* t = handle_cast<GLTexture *>(handle);
		// Note: glGenerateMimap can also fail if the internal format is not both
		// color-renderable and filterable (i.e.: doesn't work for depth)
		bindTexture(OpenGLContext::DUMMY_TEXTURE_BINDING, t);
		gl.activeTexture(OpenGLContext::DUMMY_TEXTURE_BINDING);

		glGenerateMipmap(t->gl.target);
	}

	void GLDriver::textureStorage(GLTexture* t, uint32_t width, uint32_t height, uint32_t depth) noexcept
	{
		auto& gl = m_Context;

		bindTexture(OpenGLContext::DUMMY_TEXTURE_BINDING, t);
		gl.activeTexture(OpenGLContext::DUMMY_TEXTURE_BINDING);

		switch (t->gl.target)
		{
		case GL_TEXTURE_2D:
		case GL_TEXTURE_CUBE_MAP:
			glTexStorage2D(t->gl.target, GLsizei(t->levels), t->gl.internalFormat, GLsizei(width), GLsizei(height));
			break;
        case GL_TEXTURE_3D:
        case GL_TEXTURE_2D_ARRAY: {
            glTexStorage3D(t->gl.target, GLsizei(t->levels), t->gl.internalFormat, GLsizei(width), GLsizei(height), GLsizei(depth));
            break;
        }
        case GL_TEXTURE_CUBE_MAP_ARRAY: {
            glTexStorage3D(t->gl.target, GLsizei(t->levels), t->gl.internalFormat, GLsizei(width), GLsizei(height), GLsizei(depth) * 6);
            break;
        }
        case GL_TEXTURE_2D_MULTISAMPLE:
            glTexStorage2DMultisample(t->gl.target, t->samples, t->gl.internalFormat, GLsizei(width), GLsizei(height), GL_TRUE);
            break;
        default: // cannot happen
            break;
		}

	    // textureStorage can be used to reallocate the texture at a new size
	    t->width = width;
	    t->height = height;
	    t->depth = depth;
	}

	void GLDriver::framebufferTexture(TargetBufferInfo const& binfo, GLRenderTarget const* rt, GLenum attachment, uint8_t layerCount) noexcept
	{
#if !defined(NDEBUG)
		// Only used by assert_invariant() checks below
		auto valueForLevel = [](size_t level, size_t value) {
			return std::max(size_t(1), value >> level);
		};
#endif

		GLTexture* t = handle_cast<GLTexture*>(binfo.handle);

		ASSERT(t);
		ASSERT(rt->width  <= valueForLevel(binfo.level, t->width) &&
			   rt->height <= valueForLevel(binfo.level, t->height));

		TargetBufferFlags resolveFlags = {};
		switch (attachment) {
		case GL_COLOR_ATTACHMENT0:
		case GL_COLOR_ATTACHMENT1:
		case GL_COLOR_ATTACHMENT2:
		case GL_COLOR_ATTACHMENT3:
		case GL_COLOR_ATTACHMENT4:
		case GL_COLOR_ATTACHMENT5:
		case GL_COLOR_ATTACHMENT6:
		case GL_COLOR_ATTACHMENT7:
			static_assert(MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT == 8);

			resolveFlags = getTargetBufferFlagsAt(attachment - GL_COLOR_ATTACHMENT0);
			break;
		case GL_DEPTH_ATTACHMENT:
			resolveFlags = TargetBufferFlags::DEPTH;
			break;
		case GL_STENCIL_ATTACHMENT:
			resolveFlags = TargetBufferFlags::STENCIL;
			break;
		case GL_DEPTH_STENCIL_ATTACHMENT:
			resolveFlags = TargetBufferFlags::DEPTH;
			resolveFlags |= TargetBufferFlags::STENCIL;
			break;
		default:
			break;
		}

		bool attachmentTypeNotSupportedByMSRTT = false;
		switch (attachment) {
		case GL_DEPTH_STENCIL_ATTACHMENT:
		case GL_DEPTH_ATTACHMENT:
		case GL_STENCIL_ATTACHMENT:
			attachmentTypeNotSupportedByMSRTT = rt->gl.samples != t->samples;
			break;
		default:
			break;
		}

		auto& gl = m_Context;

		GLenum target = GL_TEXTURE_2D;
		if (any(t->usage & TextureUsage::SAMPLEABLE)) {
			switch (t->target) {
			case SamplerType::SAMPLER_2D:
			case SamplerType::SAMPLER_3D:
			case SamplerType::SAMPLER_2D_ARRAY:
			case SamplerType::SAMPLER_CUBEMAP_ARRAY:
				// this could be GL_TEXTURE_2D_MULTISAMPLE or GL_TEXTURE_2D_ARRAY
				target = t->gl.target;
				// note: multi-sampled textures can't have mipmaps
				break;
			case SamplerType::SAMPLER_CUBEMAP:
				target = RHI_Internal::GetGLCubeMapFace(binfo.layer);
				// note: cubemaps can't be multi-sampled
				break;
			}
		}

		if (!(target == GL_TEXTURE_2D ||
		  	target == GL_TEXTURE_CUBE_MAP_POSITIVE_X ||
		  	target == GL_TEXTURE_CUBE_MAP_NEGATIVE_X ||
		  	target == GL_TEXTURE_CUBE_MAP_POSITIVE_Y ||
		  	target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y ||
		  	target == GL_TEXTURE_CUBE_MAP_POSITIVE_Z ||
		  	target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)) {
			attachmentTypeNotSupportedByMSRTT = true;
		}

		if (rt->gl.samples <= 1 || (rt->gl.samples > 1 && t->samples > 1))
		{
			gl.bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);

			switch (target) {
			case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
			case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
			case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
			case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
			case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
			case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
			case GL_TEXTURE_2D:
#if defined(BACKEND_OPENGL_LEVEL_GLES31)
			case GL_TEXTURE_2D_MULTISAMPLE:
#endif
				if (any(t->usage & TextureUsage::SAMPLEABLE)) {
					glFramebufferTexture2D(GL_FRAMEBUFFER, attachment,
							target, t->gl.id, binfo.level);
				} else {
					ASSERT(target == GL_TEXTURE_2D);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment,
							GL_RENDERBUFFER, t->gl.id);
				}
				break;
			case GL_TEXTURE_3D:
			case GL_TEXTURE_2D_ARRAY:
			case GL_TEXTURE_CUBE_MAP_ARRAY:
				ASSERT(layerCount == 1)
				// GL_TEXTURE_2D_MULTISAMPLE_ARRAY is not supported in GLES
				glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment,
					t->gl.id, binfo.level, binfo.layer);
				break;
			default:
				// we shouldn't be here
				break;
			}
		}
		else if (!any(t->usage & TextureUsage::SAMPLEABLE) && t->samples > 1) {
			// for render buffer. needn't sampler.
			ASSERT(rt->gl.samples > 1);
			ASSERT(glIsRenderbuffer(t->gl.id));

			// Since this attachment is not sampleable, there is no need for a sidecar or explicit
			// resolve. We can simply render directly into the renderbuffer that was allocated in
			// createTexture.
			gl.bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, t->gl.id);

			// Clear the resolve bit for this particular attachment. Note that other attachment(s)
			// might be sampleable, so this does not necessarily prevent the resolve from occurring.
			resolveFlags = TargetBufferFlags::NONE;
		} else
		{
			// for texture, ues sidecarrenderbuffer.
			// TODO: use sidecar renderbuffer
			ASSERT(0);
		}

		rt->gl.resolve |= resolveFlags;
	}

	void GLDriver::update3DImage(Handle<HwTexture> th, uint32_t level, uint32_t xoffset, uint32_t yoffset,
		uint32_t zoffset, uint32_t width, uint32_t height, uint32_t depth, PixelBufferDescriptor&& data)
	{
		setTextureData(handle_cast<GLTexture *>(th), level, xoffset, yoffset, zoffset, width, height, depth, std::move(data));
	}

	void GLDriver::renderBufferStorage(GLuint rbo, GLenum internalformat, uint32_t width, uint32_t height, uint8_t samples) const noexcept
	{
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);

		if (samples > 1)
		{
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internalformat, (GLsizei)width, (GLsizei)height);
		}
		else
		{
			glRenderbufferStorage(GL_RENDERBUFFER, internalformat, (GLsizei)width, (GLsizei)height);
		}
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	void GLDriver::updateVertexArrayObject(GLRenderPrimitive* rp, GLVertexBuffer const* vb)
	{
		auto& gl = m_Context;

#ifndef NDEBUG
		// The VAO for the given render primitive must already be bound.
		GLint vaoBinding;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vaoBinding);
		ASSERT(vaoBinding == (GLint)rp->gl.vao[gl.m_ContextIndex]);
#endif

		if ((rp->gl.vertexBufferVersion == vb->bufferObjectsVersion && rp->gl.stateVersion == gl.state.age)) {
			return;
		}

		GLVertexBufferInfo const* const vbi = handle_cast<const GLVertexBufferInfo*>(vb->vbih);

		for (size_t i = 0, n = vbi->attributes.size(); i < n; i++) {
			const auto& attribute = vbi->attributes[i];
			const uint8_t bi = attribute.buffer;
			if (bi != Attribute::BUFFER_UNUSED) {
				// if a buffer is defined it must not be invalid.
				ASSERT(vb->gl.buffers[bi]);

				// if we're on ES2, the user shouldn't use FLAG_INTEGER_TARGET
				ASSERT(attribute.flags & Attribute::FLAG_INTEGER_TARGET);

				gl.bindBuffer(GL_ARRAY_BUFFER, vb->gl.buffers[bi]);
				GLuint const index = i;
				GLint const size = RHI_Internal::GetTypeComponentCount(attribute.type);
				GLenum const type = RHI_Internal::GetTypeComponentType(attribute.type);
				GLboolean const normalized = (attribute.flags & Attribute::FLAG_NORMALIZED) ? GL_TRUE : GL_FALSE;
				GLsizei const stride = attribute.stride;
				void const* pointer = reinterpret_cast<void const *>(attribute.offset);

				if (UTILS_UNLIKELY(attribute.flags & Attribute::FLAG_INTEGER_TARGET)) {
					// integer attributes can't be floats
					ASSERT(type == GL_BYTE || type == GL_UNSIGNED_BYTE || type == GL_SHORT ||
						type == GL_UNSIGNED_SHORT || type == GL_INT || type == GL_UNSIGNED_INT);
					glVertexAttribIPointer(index, size, type, stride, pointer);
				} else
				{
					glVertexAttribPointer(index, size, type, normalized, stride, pointer);
				}

				gl.enableVertexAttribArray(&rp->gl, GLuint(i));
			} else {
				// In some OpenGL implementations, we must supply a properly-typed placeholder for
				// every integer input that is declared in the vertex shader.
				// Note that the corresponding doesn't have to be enabled and in fact won't be. If it
				// was enabled, it would indicate a user-error (providing the wrong type of array).
				// With a disabled array, the vertex shader gets the attribute from glVertexAttrib,
				// and must have the proper intergerness.
				// But at this point, we don't know what the shader requirements are, and so we must
				// rely on the attribute.

				if (UTILS_UNLIKELY(attribute.flags & Attribute::FLAG_INTEGER_TARGET)) {
					// on ES2, we know the shader doesn't have integer attributes
					glVertexAttribI4ui(GLuint(i), 0, 0, 0, 0);
				} else
				{
					glVertexAttrib4f(GLuint(i), 0, 0, 0, 0);
				}

				gl.disableVertexAttribArray(&rp->gl, GLuint(i));
			}
		}

		rp->gl.stateVersion = gl.state.age;
	}

	void GLDriver::bindTexture(GLuint unit, GLTexture const* t) noexcept
	{
		m_Context.bindTexture(unit, t->gl.target, t->gl.id, false);
	}

	void GLDriver::bindSampler(GLuint unit, GLuint sampler) noexcept
	{
		m_Context.bindSampler(unit, sampler);
	}

	bool GLDriver::useProgram(GLProgram* p) noexcept
	{
		if (mBoundProgram != p) {
			// compile/link the program if needed and call glUseProgram
			m_Context.useProgram(p->gl.program);

			decltype(mInvalidDescriptorSetBindings) changed;
			changed.setValue((1 << MAX_DESCRIPTOR_SET_COUNT) - 1);
			mInvalidDescriptorSetBindings |= changed;

			mBoundProgram = p;
		}
		return true;
	}

	void GLDriver::updateDescriptors(Util::bitset8 invalidDescriptorSets) noexcept
	{
		
	}

	void GLDriver::setTextureData(GLTexture* t, uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
								uint32_t width, uint32_t height, uint32_t depth, PixelBufferDescriptor&& p)
	{
		auto& gl = m_Context;

		ASSERT(t != nullptr);
		ASSERT(xoffset + width <= std::max(1u, t->width >> level));
		ASSERT(yoffset + height <= std::max(1u, t->height >> level));
		ASSERT(t->samples <= 1);

		GLenum glFormat = RHI_Internal::GetGLFormat(p.format);
		GLenum glType= RHI_Internal::GetGLType(p.type);

		gl.pixelStore(GL_UNPACK_ROW_LENGTH, GLint(p.stride));
		gl.pixelStore(GL_UNPACK_ALIGNMENT, GLint(p.alignment));

		using PBD = PixelBufferDescriptor;
	    size_t const stride = p.stride ? p.stride : width;
	    size_t const bpp = PBD::computeDataSize(p.format, p.type, 1, 1, 1);
	    size_t const bpr = PBD::computeDataSize(p.format, p.type, stride, 1, p.alignment);
	    size_t const bpl = bpr * height; // TODO: PBD should have a "layer stride"
	    void const* const buffer = static_cast<char const*>(p.buffer)
	            + bpp* p.left + bpr * p.top + bpl * 0; // TODO: PBD should have a p.depth

	    switch (t->target) {
	        case SamplerType::SAMPLER_2D:
	            // NOTE: GL_TEXTURE_2D_MULTISAMPLE is not allowed
	            bindTexture(OpenGLContext::DUMMY_TEXTURE_BINDING, t);
	            gl.activeTexture(OpenGLContext::DUMMY_TEXTURE_BINDING);
	            ASSERT(t->gl.target == GL_TEXTURE_2D);
	            glTexSubImage2D(t->gl.target, GLint(level),
	                    GLint(xoffset), GLint(yoffset),
	                    GLsizei(width), GLsizei(height), glFormat, glType, buffer);
	            break;
	        case SamplerType::SAMPLER_3D:
	            ASSERT(zoffset + depth <= std::max(1u, t->depth >> level));
	            bindTexture(OpenGLContext::DUMMY_TEXTURE_BINDING, t);
	            gl.activeTexture(OpenGLContext::DUMMY_TEXTURE_BINDING);
	            ASSERT(t->gl.target == GL_TEXTURE_3D);
	            glTexSubImage3D(t->gl.target, GLint(level),
	                    GLint(xoffset), GLint(yoffset), GLint(zoffset),
	                    GLsizei(width), GLsizei(height), GLsizei(depth), glFormat, glType, buffer);
	            break;
	        case SamplerType::SAMPLER_2D_ARRAY:
	        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
	            ASSERT(zoffset + depth <= t->depth);
	            // NOTE: GL_TEXTURE_2D_MULTISAMPLE is not allowed
	            bindTexture(OpenGLContext::DUMMY_TEXTURE_BINDING, t);
	            gl.activeTexture(OpenGLContext::DUMMY_TEXTURE_BINDING);
	            ASSERT(t->gl.target == GL_TEXTURE_2D_ARRAY ||
	                    t->gl.target == GL_TEXTURE_CUBE_MAP_ARRAY);
	            glTexSubImage3D(t->gl.target, GLint(level),
	                    GLint(xoffset), GLint(yoffset), GLint(zoffset),
	                    GLsizei(width), GLsizei(height), GLsizei(depth), glFormat, glType, buffer);
	            break;
	        case SamplerType::SAMPLER_CUBEMAP: {
	            ASSERT(t->gl.target == GL_TEXTURE_CUBE_MAP);
	            bindTexture(OpenGLContext::DUMMY_TEXTURE_BINDING, t);
	            gl.activeTexture(OpenGLContext::DUMMY_TEXTURE_BINDING);

	            ASSERT(width == height);
	            const size_t faceSize = PixelBufferDescriptor::computeDataSize(
	                    p.format, p.type, p.stride ? p.stride : width, height, p.alignment);
	            ASSERT(zoffset + depth <= 6);
	            for (size_t face = 0; face < depth; face++) {
	                GLenum const target = RHI_Internal::GetGLCubeMapFace(zoffset + face);
	                glTexSubImage2D(target, GLint(level), GLint(xoffset), GLint(yoffset),
	                        GLsizei(width), GLsizei(height), glFormat, glType,
	                        static_cast<uint8_t const*>(buffer) + faceSize * face);
	            }
	            break;
	        }
	    }
	}
}


