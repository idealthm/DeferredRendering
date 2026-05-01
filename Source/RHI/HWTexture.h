#pragma once

#include "Common/Core.h"
#include "RHI/RHITypes.h"

class HWTexture
{
public:
	static Ref<HWTexture> Create(const RHI::TextureDesc& desc, const void* data = nullptr);

	virtual ~HWTexture() = default;

	virtual void Bind(uint32_t slot = 0) = 0;
	virtual void Unbind() = 0;
	virtual uint32_t GetRendererID() const = 0;
	virtual void GenerateMipmap() = 0;

	virtual uint32_t GetSizeX() const = 0;
	virtual uint32_t GetSizeY() const = 0;

	virtual void SetData(const void* data, uint32_t size) {}
	virtual void SetFaceData(uint32_t srcName, uint32_t srcTarget, int32_t srcLevel,
		int32_t srcX, int32_t srcY, int32_t srcZ, uint32_t face) {}
};
