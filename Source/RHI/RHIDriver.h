#pragma once

#include <string>

#include "DriverEnums.h"
#include "Common/HandleAllocator.h"
#include "RHI/DescriptorSet.h"

class PixelBufferDescriptor;
class BufferDescriptor;
class Program;

namespace RHI
{
	struct PipelineState;
	struct TargetBufferInfo;
	class MRT;

struct HwBase {
};

struct HwVertexBufferInfo : public HwBase
{
	uint8_t bufferCount{};                //   1
	uint8_t attributeCount{};             //   1
	bool padding[2]{};                    //   2
	HwVertexBufferInfo() noexcept = default;
	HwVertexBufferInfo(uint8_t bufferCount, uint8_t attributeCount) noexcept
			: bufferCount(bufferCount),
			  attributeCount(attributeCount) {
	}
};

struct HwVertexBuffer : public HwBase
{
	uint32_t vertexCount{};               //   4
	uint16_t bufferObjectsVersion{0xff};  //   2
	uint16_t padding0;					  //   2
	HwVertexBuffer() noexcept = default;
	explicit HwVertexBuffer(uint32_t vertextCount) noexcept
			: vertexCount(vertextCount) {
	}
};

struct HwIndexBuffer : public HwBase
{
	uint32_t count : 27;
	uint32_t elementSize : 5;

	HwIndexBuffer() noexcept : count{}, elementSize{} { }
	HwIndexBuffer(uint32_t indexCount, uint8_t elementSize) noexcept :
			count(indexCount), elementSize(elementSize) {
		// we could almost store elementSize on 4 bits because it's never > 16 and never 0
		ASSERT(elementSize > 0 && elementSize <= 16);
		ASSERT(indexCount < (1u << 27));
	}
};

struct HwBufferObject : public HwBase
{
	uint32_t byteCount{};

	HwBufferObject() noexcept = default;
	explicit HwBufferObject(uint32_t byteCount) noexcept : byteCount(byteCount) {}
};

struct HwRenderPrimitive : public HwBase
{
	PrimitiveType type = PrimitiveType::TRIANGLES;
};

struct HwProgram : public HwBase
{
	std::string name;
	explicit HwProgram(const std::string& name) noexcept : name(std::move(name)) { }
	HwProgram() noexcept = default;
};

struct HwDescriptorSetLayout : public HwBase
{
	HwDescriptorSetLayout() noexcept = default;
};

struct HwDescriptorSet : public HwBase
{
	HwDescriptorSet() noexcept = default;
};

struct HwRenderTarget : public HwBase
{
	uint32_t width{};
	uint32_t height{};
	HwRenderTarget() noexcept = default;
	HwRenderTarget(uint32_t w, uint32_t h) : width(w), height(h) { }
};

struct HwTexture : public HwBase
{
	uint32_t width{};
	uint32_t height{};
	uint32_t depth{};
	SamplerType target{};
	uint8_t levels : 4;  // This allows up to 15 levels (max texture size of 32768 x 32768)
	uint8_t samples : 4; // Sample count per pixel (should always be a power of 2)
	Format format{};
	uint8_t reserved0 = 0;
	TextureUsage usage{};
	uint16_t reserved1 = 0;

	HwTexture() noexcept : levels{}, samples{} {}
	HwTexture(SamplerType target, uint8_t levels, uint8_t samples,
			  uint32_t width, uint32_t height, uint32_t depth, Format fmt, TextureUsage usage) noexcept
			: width(width), height(height), depth(depth),
			  target(target), levels(levels), samples(samples), format(fmt), usage(usage) { }
};


class RHIDriver
{
public:
	RHIDriver() = default;
	RHIDriver(const RHIDriver&) = default;
	RHIDriver& operator=(const RHIDriver&) = default;

	virtual ~RHIDriver() = default;

	virtual void draw2(size_t offset, size_t  count, uint32_t instanceCount) = 0;
	virtual void draw(PipelineState state, Handle<HwRenderPrimitive> rph, uint32_t indexOffset, uint32_t indexCount, uint32_t instanceCount) = 0;
	virtual void bindPipeline(PipelineState const& state) = 0;
	virtual void bindRenderPrimitive(Handle<HwRenderPrimitive> rph) = 0;

	virtual void CommitDescriptorSet(const DescriptorSet& ds) {}
	virtual void beginRenderPass(Handle<HwRenderTarget> h, RenderPassParams& params) = 0;
	virtual void endRenderPass() = 0;

	// --- Backend-specific factory methods sync---------------------------------
	virtual Handle<HwProgram> CreateProgram(Program&& program) = 0;
	virtual Handle<HwTexture> CreateTexture(SamplerType target, uint8_t levels, Format format, uint8_t samples, uint32_t width, uint32_t height, uint32_t depth, TextureUsage usage) = 0;
	virtual Handle<HwTexture> CreateTextureView(Handle<HwTexture> srcth, uint8_t baseLevel, uint8_t maxLevel) = 0;
	virtual Handle<HwBufferObject> CreateBufferObject(size_t size, BufferObjectBinding target, BufferUsage usage) = 0;
	virtual Handle<HwVertexBufferInfo> CreateVertexBufferInfo(size_t bufferCount, size_t attributeCount, AttributeArray) = 0;
	virtual Handle<HwVertexBuffer> CreateVertexBuffer(size_t vertexCount, Handle<HwVertexBufferInfo> info) = 0;
	virtual Handle<HwIndexBuffer> CreateIndexBuffer(ElementType type, size_t indexCount, BufferUsage usage) = 0;
	virtual Handle<HwRenderPrimitive> CreateRenderPrimitive(Handle<HwVertexBuffer> vbh, Handle<HwIndexBuffer> ibh, PrimitiveType type) = 0;
	virtual Handle<HwRenderTarget> CreateRenderTarget(TargetBufferFlags targets, uint32_t width, uint32_t height, uint8_t samplers, uint8_t layerCount,
		MRT color, TargetBufferInfo depth, TargetBufferInfo stencil) = 0;
	virtual Handle<HwDescriptorSet> CreateDescriptorSet(Handle<HwDescriptorSetLayout> dslh) = 0;
	virtual Handle<HwDescriptorSetLayout> CreateDescriptorSetLayout(DescriptorSetLayout&& info) = 0;

	virtual void CreateProgram(Handle<HwProgram>, Program&& program)  = 0;
	virtual void CreateTexture(Handle<HwTexture>, SamplerType target, uint8_t levels, Format format, uint8_t samples, uint32_t width, uint32_t height, uint32_t depth, TextureUsage usage)  = 0;
	virtual void CreateTextureView(Handle<HwTexture> h, Handle<HwTexture> srcth, uint8_t baseLevel, uint8_t maxLevel) = 0;
	virtual void CreateBufferObject(Handle<HwBufferObject>, size_t size, BufferObjectBinding target, BufferUsage usage)  = 0;
	virtual void CreateVertexBufferInfo(Handle<HwVertexBufferInfo>, size_t bufferCount, size_t attributeCount, AttributeArray attributes) = 0;
	virtual void CreateVertexBuffer(Handle<HwVertexBuffer>, size_t vertexCount, Handle<HwVertexBufferInfo> info)  = 0;
	virtual void CreateIndexBuffer(Handle<HwIndexBuffer>, ElementType type, size_t indexCount, BufferUsage usage)  = 0;
	virtual void CreateRenderPrimitive(Handle<HwRenderPrimitive>, Handle<HwVertexBuffer> vbh, Handle<HwIndexBuffer> ibh, PrimitiveType type)  = 0;
	virtual void CreateRenderTarget(Handle<HwRenderTarget>, TargetBufferFlags targets, uint32_t width, uint32_t height, uint8_t samplers, uint8_t layerCount, MRT color, TargetBufferInfo depth, TargetBufferInfo stencil)  = 0;
	virtual void CreateDescriptorSet(Handle<HwDescriptorSet>, Handle<HwDescriptorSetLayout> dslh)  = 0;
	virtual void CreateDescriptorSetLayout(Handle<HwDescriptorSetLayout>, DescriptorSetLayout&& info)  = 0;

	// --- Backend-specific factory methods sync---------------------------------
	virtual void DestroyProgram(Handle<HwProgram>) = 0;
	virtual void DestroyTexture(Handle<HwTexture>) = 0;
	virtual void DestroyBuffer(Handle<HwBufferObject>) = 0;
	virtual void DestroyVertexBufferInfo(Handle<HwVertexBufferInfo>) = 0;
	virtual void DestroyVertexBuffer(Handle<HwVertexBuffer>) = 0;
	virtual void DestroyIndexBuffer(Handle<HwIndexBuffer>) = 0;
	virtual void DestroyRenderPrimitive(Handle<HwRenderPrimitive>) = 0;
	virtual void DestroyRenderTarget(Handle<HwRenderTarget>) = 0;
	virtual void DestroyDescriptorSet(Handle<HwDescriptorSet>) = 0;
	virtual void DestroyDescriptorSetLayout(Handle<HwDescriptorSetLayout>) = 0;
	
	// --- Buffer data management -------------------------------------------

	virtual void updateDescriptorSetTexture(Handle<HwDescriptorSet> dsh, descriptor_binding_t binding, Handle<HwTexture> h, SamplerParams& params) = 0;
	virtual void updateDescriptorSetBuffer(Handle<HwDescriptorSet> dsh, descriptor_binding_t binding, Handle<HwBufferObject> h, uint16_t offset, uint16_t size) = 0;
	virtual void bindDescriptorSet(Handle<HwDescriptorSet> h, uint8_t setIndex) = 0;
	virtual void updateBufferObject(Handle<HwBufferObject> boh, BufferDescriptor&& data, uint32_t byteOffset = 0) = 0;
	virtual void setVertexBufferObject(Handle<HwVertexBuffer> vbh, uint8_t bufferSlot, Handle<HwBufferObject> boh) = 0;
	virtual void setIndexBufferObject(Handle<HwIndexBuffer> ibh, Handle<HwBufferObject> boh) = 0;

	// --- Global state -----------------------------------------------------

	virtual void update3DImage(Handle<HwTexture> th, uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
		uint32_t width, uint32_t height, uint32_t depth, PixelBufferDescriptor&& data) = 0;
	virtual void SetClearColor(float r, float g, float b, float a) = 0;
	virtual void SetViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h) = 0;
	virtual void Init() = 0;

	virtual void generateMipmap(Handle<HwTexture> handle) = 0;
};

} // namespace RHI
