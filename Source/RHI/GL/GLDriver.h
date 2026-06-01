#pragma once

#include <array>
#include <bitset>
#include <iostream>

#include "RHI/RHIDriver.h"
#include "GLBufferObject.h"
#include "GLTexture.h"
#include "OpenGLContext.h"
#include "RHI/DriverEnums.h"
#include "RHI/TargetBufferInfo.h"
#include "RHI/DescriptorSet.h"
#include "Shader/Program.h"

class PixelBufferDescriptor;
class GLProgram;


namespace
{
	void OpenGLMessageCallback(
		unsigned source,
		unsigned type,
		unsigned id,
		unsigned severity,
		int length,
		const char* message,
		const void* userParam)
	{
		ASSERT(0);
		std::cout << message << std::endl;
	}
}

namespace RHI
{
	struct GLDescriptorSet;
	struct GLDescriptorSetLayout;
	class OpenGLContext;

	struct GLVertexBufferInfo : public HwVertexBufferInfo {
        GLVertexBufferInfo() noexcept = default;
        GLVertexBufferInfo(uint8_t bufferCount, uint8_t attributeCount,
                AttributeArray const& attributes)
                : HwVertexBufferInfo(bufferCount, attributeCount),
                  attributes(attributes) {
        }
        AttributeArray attributes;
    };

    struct GLVertexBuffer : public HwVertexBuffer {
        GLVertexBuffer() noexcept = default;
        GLVertexBuffer(uint32_t vertexCount, Handle<HwVertexBufferInfo> vbih)
                : HwVertexBuffer(vertexCount), vbih(vbih) {
        }
        Handle<HwVertexBufferInfo> vbih;
        struct {
            // 4 * MAX_VERTEX_ATTRIBUTE_COUNT bytes
            std::array<GLuint, MAX_VERTEX_ATTRIBUTE_COUNT> buffers{};
        } gl;
    };

    struct GLIndexBuffer : public HwIndexBuffer {
        using HwIndexBuffer::HwIndexBuffer;
        struct {
            GLuint buffer{};
        } gl;
    };

    struct GLRenderPrimitive : public HwRenderPrimitive {
        using HwRenderPrimitive::HwRenderPrimitive;
        OpenGLContext::RenderPrimitive gl;
        Handle<HwVertexBufferInfo> vbih;
    };

    struct GLRenderTarget : public HwRenderTarget {
        using HwRenderTarget::HwRenderTarget;
        struct {
            // field ordering to optimize size on 64-bits
            GLTexture* color[MAX_SUPPORTED_RENDER_TARGET_COUNT];
            GLTexture* depth;
            GLTexture* stencil;
            GLuint fbo = 0;
            mutable GLuint fbo_read = 0;
            mutable TargetBufferFlags resolve = TargetBufferFlags::NONE; // attachments in fbo_draw to resolve
            uint8_t samples = 1;
            bool isDefault = false;
        } gl;
        TargetBufferFlags targets = {};
    };

	class GLDriver : public RHI::RHIDriver
	{
	public:

		GLDriver();

		template<typename D, typename... Args>
		Handle<D> InitHandle(Args&&... args)
		{
			return m_HandleAllocator.allocateAndConstruct<D>(std::forward<Args>(args)...);
		}

		template<typename D, typename B, typename... Args>
		std::enable_if_t<std::is_base_of_v<B, D>, D>* construct(Handle<B> handle, Args&&... args)
		{
			return m_HandleAllocator.destroyAndConstruct<D>(handle, std::forward<Args>(args)...);
		}

		template<typename B, typename D, typename = std::enable_if_t<std::is_base_of_v<B, D>, D>>
		void destruct(Handle<B>& handle, D const* p) noexcept {
			return m_HandleAllocator.deallocate(handle, p);
		}

		template<typename Dp, typename B>
		std::enable_if_t< std::is_pointer_v<Dp> && std::is_base_of_v<B, std::remove_pointer_t<Dp>>, Dp>
		handle_cast(Handle<B>& handle) const {
			return m_HandleAllocator.handle_cast<Dp, B>(handle);
		}

		template<typename Dp, typename B>
		std::enable_if_t< std::is_pointer_v<Dp> && std::is_base_of_v<B, std::remove_pointer_t<Dp>>, Dp>
		handle_cast(Handle<B> const& handle) const {
			return m_HandleAllocator.handle_cast<Dp, B>(handle);
		}

		template<typename B>
		bool is_valid(Handle<B>& handle) {
			return m_HandleAllocator.is_valid(handle);
		}

		using AttachmentArray = std::array<GLenum, MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT + 2>;

		// Resource

		virtual Handle<HwProgram> CreateProgram(Program&& program) override;
		virtual Handle<HwTexture> CreateTexture(SamplerType target, uint8_t levels, Format format, uint8_t samples, uint32_t width, uint32_t height, uint32_t depth, TextureUsage usage) override;
		virtual Handle<HwTexture> CreateTextureView(Handle<HwTexture> srcth, uint8_t baseLevel, uint8_t levelCount) override;
		virtual Handle<HwBufferObject> CreateBufferObject(size_t size, BufferObjectBinding target, BufferUsage usage) override;
		virtual Handle<HwVertexBufferInfo> CreateVertexBufferInfo(size_t bufferCount, size_t attributeCount, const AttributeArray& attributes) override;
		virtual Handle<HwVertexBuffer> CreateVertexBuffer(size_t vertexCount, Handle<HwVertexBufferInfo> info) override;
		virtual Handle<HwIndexBuffer> CreateIndexBuffer(ElementType type, size_t indexCount, BufferUsage usage) override;
		virtual Handle<HwRenderPrimitive> CreateRenderPrimitive(Handle<HwVertexBuffer> vbh, Handle<HwIndexBuffer> ibh, PrimitiveType type) override;
		virtual Handle<HwRenderTarget> CreateRenderTarget(TargetBufferFlags targets, uint32_t width, uint32_t height, uint8_t samplers, uint8_t layerCount, MRT color, TargetBufferInfo depth, TargetBufferInfo stencil) override;
		virtual Handle<HwDescriptorSet> CreateDescriptorSet(Handle<HwDescriptorSetLayout> dslh) override;
		virtual Handle<HwDescriptorSetLayout> CreateDescriptorSetLayout(DescriptorSetLayout&& info) override;

		virtual void CreateProgram(Handle<HwProgram> h, Program&& program) override;
		virtual void CreateTexture(Handle<HwTexture> h, SamplerType target, uint8_t levels, Format format, uint8_t samples, uint32_t width, uint32_t height, uint32_t depth, TextureUsage usage) override;
		virtual void CreateTextureView(Handle<HwTexture> h, Handle<HwTexture> srcth, uint8_t baseLevel, uint8_t levelCount) override;
		virtual void CreateBufferObject(Handle<HwBufferObject> h, size_t count, BufferObjectBinding target, BufferUsage usage) override;
		virtual void CreateVertexBufferInfo(Handle<HwVertexBufferInfo> h, size_t bufferCount, size_t attributeCount, const AttributeArray& attributes) override;
		virtual void CreateVertexBuffer(Handle<HwVertexBuffer> h, size_t vertexCount, Handle<HwVertexBufferInfo> info) override;
		virtual void CreateIndexBuffer(Handle<HwIndexBuffer> h, ElementType type, size_t indexCount, BufferUsage usage) override;
		virtual void CreateRenderPrimitive(Handle<HwRenderPrimitive> h, Handle<HwVertexBuffer> vbh, Handle<HwIndexBuffer> ibh, PrimitiveType type) override;
		virtual void CreateRenderTarget(Handle<HwRenderTarget> h, TargetBufferFlags targets, uint32_t width, uint32_t height, uint8_t samples, uint8_t layerCount, MRT color, TargetBufferInfo depth, TargetBufferInfo stencil) override;
		virtual void CreateDescriptorSet(Handle<HwDescriptorSet> h, Handle<HwDescriptorSetLayout> dslh) override;
		virtual void CreateDescriptorSetLayout(Handle<HwDescriptorSetLayout> h, DescriptorSetLayout&& info) override;

		virtual void DestroyProgram(Handle<HwProgram> h) override;
		virtual void DestroyTexture(Handle<HwTexture> h) override;
		virtual void DestroyBuffer(Handle<HwBufferObject> h) override;
		virtual void DestroyVertexBufferInfo(Handle<HwVertexBufferInfo> h) override;
		virtual void DestroyVertexBuffer(Handle<HwVertexBuffer> h) override;
		virtual void DestroyIndexBuffer(Handle<HwIndexBuffer> h) override;
		virtual void DestroyRenderPrimitive(Handle<HwRenderPrimitive> h) override;
		virtual void DestroyRenderTarget(Handle<HwRenderTarget> h) override;
		virtual void DestroyDescriptorSet(Handle<HwDescriptorSet> h) override;
		virtual void DestroyDescriptorSetLayout(Handle<HwDescriptorSetLayout> h) override;

		// Buffer data management
		virtual void updateDescriptorSetBuffer(Handle<HwDescriptorSet> dsh, descriptor_binding_t binding, Handle<HwBufferObject> h, uint16_t offset, uint16_t size) override;
		virtual void updateDescriptorSetTexture(Handle<HwDescriptorSet> dsh, descriptor_binding_t binding, Handle<HwTexture> h, SamplerParams& params) override;
		virtual void bindDescriptorSet(Handle<HwDescriptorSet> h, uint8_t set) override;
		virtual void updateBufferObject(Handle<HwBufferObject> boh, BufferDescriptor&& data, uint32_t byteOffset = 0) override;
		virtual void setVertexBufferObject(Handle<HwVertexBuffer> vbh, uint8_t bufferSlot, Handle<HwBufferObject> boh) override;
		virtual void setIndexBufferObject(Handle<HwIndexBuffer> ibh, Handle<HwBufferObject> boh) override;

		// Misc
		uint32_t getMaxDrawBuffers() const;
		OpenGLContext& GetContext() {return m_Context;}
		bool CompileShader(const Program::ShaderSource& src, uint32_t& program) const;

		void generateMipmap(Handle<HwTexture> handle) override;
		uint32_t GetNativeTextureId(Handle<HwTexture> h) const override;

		//
		void beginRenderPass(Handle<HwRenderTarget> h, RenderPassParams& params) override;
		void endRenderPass() override;

		GLsizei getAttachments(AttachmentArray& attachments, TargetBufferFlags buffers, bool isDefaultFramebuffer) noexcept;

		void draw2(size_t offset, size_t count, uint32_t instanceCount) override;
		void draw(PipelineState state, Handle<HwRenderPrimitive> rph, uint32_t indexOffset, uint32_t indexCount, uint32_t instanceCount) override;
		void bindPipeline(PipelineState const& state) override;
		void bindRenderPrimitive(Handle<HwRenderPrimitive> rph) override;

		void clearWithRasterPipe(TargetBufferFlags clearFlags,
			glm::vec4 const& linearColor, GLfloat depth, GLint stencil) noexcept;

		//
		void setRasterState(RasterState rs) noexcept;
		void setStencilState(StencilState ss) noexcept;

		void SetClearColor(float r, float g, float b, float a) override
		{
			glClearColor(r, g, b, a);
		}

		void SetViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h) override
		{
			glViewport(x, y, w, h);
		}

		void updateVertexArrayObject(GLRenderPrimitive* rp, GLVertexBuffer const* vb);

		void framebufferTexture(TargetBufferInfo const& binfo,
				GLRenderTarget const* rt, GLenum attachment, uint8_t layerCount) noexcept;

		void update3DImage(Handle<HwTexture> th, uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
			uint32_t width, uint32_t height, uint32_t depth, PixelBufferDescriptor&& data) override;

		void setTextureData(GLTexture* t,
				uint32_t level,
				uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
				uint32_t width, uint32_t height, uint32_t depth,
				PixelBufferDescriptor&& p);

		void renderBufferStorage(GLuint rbo, GLenum internalformat, uint32_t width, uint32_t height, uint8_t samples) const noexcept;

		void textureStorage(GLTexture* t, uint32_t width, uint32_t height,uint32_t depth) noexcept;

		/* State tracking GL wrappers... */

		void bindTexture(GLuint unit, GLTexture const* t) noexcept;
		void bindSampler(GLuint unit, GLuint sampler) noexcept;
		inline bool useProgram(GLProgram* p) noexcept;

		void updateDescriptors(Util::bitset8 invalidDescriptorSets) noexcept;

		void Init() override
		{
#ifdef _DEBUG
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(OpenGLMessageCallback, nullptr);

			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif

			glEnable(GL_LINE_SMOOTH);
			glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		}

	private:
		struct {
			DescriptorSetHandle dsh;
			std::array<uint32_t, 9> offsets;
		} mBoundDescriptorSets[MAX_DESCRIPTOR_SET_COUNT];

		bool mValidProgram = false;

		Handle<HwRenderTarget> mRenderPassTarget;
		RenderPassParams mRenderPassParams;

		GLRenderPrimitive const* mBoundRenderPrimitive = nullptr;
		GLProgram* mBoundProgram = nullptr;
		Util::bitset8 mInvalidDescriptorSetBindings;
		Util::bitset8 mInvalidDescriptorSetBindingOffsets;
		GLboolean mRenderPassColorWrite{};
		GLboolean mRenderPassDepthWrite{};
		GLboolean mRenderPassStencilWrite{};

		OpenGLContext m_Context;
		HandleAllocatorGL m_HandleAllocator;
	};
}