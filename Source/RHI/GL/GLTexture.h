#pragma once

#include "RHI/HWTexture.h"

class GLTexture : public HWTexture
{
public:
	GLTexture(const RHI::TextureDesc& desc, const void* data);
	~GLTexture() override;

	void Bind(uint32_t slot = 0) override;
	void Unbind() override;
	uint32_t GetRendererID() const override;
	void GenerateMipmap() override;

	uint32_t GetSizeX() const override;
	uint32_t GetSizeY() const override;

	void SetData(const void* data, uint32_t size) override;
	void SetFaceData(uint32_t srcName, uint32_t srcTarget, int32_t srcLevel,
		int32_t srcX, int32_t srcY, int32_t srcZ, uint32_t face) override;

private:
	uint32_t GetGLTarget() const;

	RHI::TextureDesc m_Desc;
	uint32_t m_RendererID = 0xFFFFFFFF;
};
