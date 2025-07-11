#pragma once

#include <string>
#include <common/Core.h>

class Texture
{
public:
	virtual ~Texture() = default;

	virtual void Bind(uint32 slot) = 0;
	virtual void Unbind() = 0;

	virtual uint32 GetSizeX() = 0;
	virtual uint32 GetSizeY() = 0;
	virtual uint32 GetSizeZ() = 0;

	virtual uint32 GetRendererID() = 0;

	virtual void SetData(const void* data, uint32 size) = 0;
};

struct TextureDescription
{
	uint32 width, height;
	uint32 slice;
	uint32 format;
	uint32 bpp;
	bool SRGB;
	bool Mipmap;
	std::string importPath;
};

class Texture2D : public Texture
{
public:
	Texture2D(const TextureDescription& desc, const void* data);

	virtual ~Texture2D();

	virtual void Bind(uint32 slot = 0) override;
	virtual void Unbind() override;

	virtual uint32 GetSizeX() override;
	virtual uint32 GetSizeY() override;
	virtual uint32 GetSizeZ() override;

	virtual uint32 GetRendererID() override;

	virtual void SetData(const void* data, uint32 size) override;

	const TextureDescription& GetDesc();

public:
	static Ref<Texture2D> Create(const TextureDescription& desc, const void* data); 
	static Ref<Texture2D> Create(const std::string& path); 
private:
	uint32 m_RendererID;
	TextureDescription m_Desc;
};