#pragma once

#include <string>
#include <common/Core.h>

enum class EFBTextureFormat;

enum class ETextureWrapMode
{
	EClampToEdge,
	ECLampToBorder,
	EMirroredRepeat,
	ERepeat,
	EMirrorClampToEdge,
};

enum class ETextureFormat {
	None,
	// 颜色格式
	R8,
	RG8,
	RG16F,
	RGB8,
	RGBA8,
	SRGB8,
	SRGBA8,     // 颜色贴图必备
	RGB16F,
	RGBA16F,    // HDR/帧缓冲必备
	R11G11B10F, // 紧凑型 HDR
	// 深度格式
	Depth24,
	Depth32F,
	Depth24Stencil8
};

enum class ETextureFilter {
	Nearest,
	Linear,
	LinearMipmapLinear // 只有 MinFilter 会用到
};

static bool IsFloatFormat(ETextureFormat format);

extern uint32 GetGLWrapMode(ETextureWrapMode mode);

struct TextureDescription
{
	uint32 width, height;
	uint32 slice, MipLevel;
	ETextureFormat format;
	bool SRGB;
	bool bGenerateMipmap;
	ETextureWrapMode FilterS;
	ETextureWrapMode FilterT;
	ETextureWrapMode FilterR;
	ETextureFilter minFilter;
	ETextureFilter magFilter;
	glm::vec4 BorderColor;

	TextureDescription()
	{
		width = height = 0;
		slice = MipLevel = 0;
		format = ETextureFormat::None;
		SRGB = false;
		bGenerateMipmap = false;
		FilterS = ETextureWrapMode::EClampToEdge;
		FilterT = ETextureWrapMode::EClampToEdge;
		FilterR = ETextureWrapMode::EClampToEdge;
		minFilter = ETextureFilter::LinearMipmapLinear;
		magFilter = ETextureFilter::Linear;
		BorderColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	bool operator==(const TextureDescription& other) const
	{
		return width == other.width && height == other.height && slice == other.slice && MipLevel == other.MipLevel &&
			format == other.format && bGenerateMipmap == other.bGenerateMipmap && SRGB == other.SRGB;
	}

	bool operator!=(const TextureDescription& other) const
	{
		return !(*this == other);
	}
};

class Texture
{
public:
	virtual ~Texture();

	bool IsValid() const {return m_RendererID != 0xFFFFFFFF;}

	virtual void Init(const TextureDescription& desc, const void* data);
	virtual void SetTextureParameter(const TextureDescription& desc) const;

	virtual void Bind(uint32 slot = 0);
	virtual void Unbind();

	virtual uint32 GetSizeX() { return m_Desc.width;}
	virtual uint32 GetSizeY() { return m_Desc.height;}
	virtual uint32 GetSizeZ() { return m_Desc.slice;}

	virtual uint32 GetRendererID() { return m_RendererID; };

protected:
	TextureDescription	m_Desc;
	uint32 m_RendererID = 0xFFFFFFFF;
};


namespace 
{
	TextureDescription CreateShadowMap(uint32 size) // 通常是正方形，如 2048
	{
		TextureDescription result;
		result.width = size;
		result.height = size;
		result.slice = 1;
		result.MipLevel = 1;
		// 阴影建议用 Depth32F 或 Depth24，视精度需求而定
		result.format = ETextureFormat::Depth24Stencil8; 
		result.bGenerateMipmap = false;
		result.SRGB = false;
		// 核心：ClampToBorder 并配合白色（1.0），确保光照范围外不会有阴影
		result.FilterS = ETextureWrapMode::ECLampToBorder;
		result.FilterT = ETextureWrapMode::ECLampToBorder;
		result.BorderColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		result.minFilter = ETextureFilter::Linear; // 开启线性过滤可实现硬件级 PCF 采样
		result.magFilter = ETextureFilter::Linear;
		return result;
	}

	TextureDescription CreateFinalColor(uint32 w, uint32 h)
	{
		TextureDescription result;
		result.width = w;
		result.height = h;
		result.slice = 1;
		result.MipLevel = 1;
		result.format = ETextureFormat::SRGBA8;
		result.bGenerateMipmap = false;
		result.SRGB = true;
		result.FilterS = ETextureWrapMode::EClampToEdge;
		result.FilterT = ETextureWrapMode::EClampToEdge;
		result.minFilter = ETextureFilter::Linear;
		result.magFilter = ETextureFilter::Linear;
		return result;
	}

	TextureDescription CreateGBuffer(uint32 w, uint32 h, ETextureFormat format, bool isColor = false)
	{
		TextureDescription result;
		result.width = w;
		result.height = h;
		result.slice = 1;
		result.MipLevel = 1;
		result.format = format;
		result.bGenerateMipmap = false;
		result.SRGB = isColor; // 只有颜色附件需要 SRGB 转换
		result.FilterS = ETextureWrapMode::EClampToEdge; // FBO 附件严禁使用 Repeat
		result.FilterT = ETextureWrapMode::EClampToEdge;
		result.minFilter = ETextureFilter::Linear;
		result.magFilter = ETextureFilter::Linear;
		return result;
	}

	TextureDescription CreateSkybox(uint32 w, uint32 h)
	{
		TextureDescription result;
		result.width = w;
		result.height = h;
		result.slice = 1; // 如果是 CubeMap，逻辑会有所不同，这里假设是 2D 全景图
		result.MipLevel = 1;
		result.format = ETextureFormat::RGBA16F; // HDR 天空盒
		result.bGenerateMipmap = false;
		result.SRGB = false;
		result.FilterS = ETextureWrapMode::EClampToEdge;
		result.FilterT = ETextureWrapMode::EClampToEdge;
		result.FilterR = ETextureWrapMode::EClampToEdge;
		result.minFilter = ETextureFilter::Linear;
		result.magFilter = ETextureFilter::Linear;
		return result;
	}

	TextureDescription CreateHDRBuffer(uint32 w, uint32 h)
	{
		TextureDescription result;
		result.width = w;
		result.height = h;
		result.slice = 1;
		result.MipLevel = 1;
		result.format = ETextureFormat::RGBA16F; // 必须支持超过 1.0 的数值
		result.bGenerateMipmap = false;
		result.SRGB = false; // 在线性空间计算，最后显示时再转 SRGB
		result.FilterS = ETextureWrapMode::EClampToEdge;
		result.FilterT = ETextureWrapMode::EClampToEdge;
		result.minFilter = ETextureFilter::Linear;
		result.magFilter = ETextureFilter::Linear;
		return result;
	}

	TextureDescription CreateDepth(uint32 width, uint32 height)
	{
		TextureDescription result;
		result.width = width;
		result.height = height;
		result.slice = 1;
		result.format = ETextureFormat::Depth24Stencil8;
		result.bGenerateMipmap = false;
		result.SRGB = false;
		result.FilterS = ETextureWrapMode::ECLampToBorder;
		result.FilterT = ETextureWrapMode::ECLampToBorder;
		result.BorderColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		return result;
	}
}

struct GLFormatInfo
{
	int32 internalFormat;
	uint32 format;
	uint32 type;
};

static GLFormatInfo GetGLInfo(ETextureFormat format);

class Texture2D : public Texture
{
public:
	Texture2D(const std::string& path, bool bSRGB = false);

	Texture2D(const TextureDescription& desc, const void* data=nullptr);

	void Init(const TextureDescription& desc, const void* data) override;
	void SetTextureParameter(const TextureDescription& desc) const override;

	const TextureDescription& GetDesc();

public:
	static Ref<Texture2D> Create(const std::string& path, bool bSRGB = false);
	static Ref<Texture2D> Create(const uint32& rgba);
private:
	TextureDescription m_Desc;
};

class Texture3D : public Texture
{
public:
	Texture3D();
	Texture3D(const TextureDescription& desc, const void* data);

	const TextureDescription& GetDesc() const;

private:
	TextureDescription m_Desc;
};

class TextureCube : public Texture
{
public:
	enum CubeFace
	{
		Right,
		Left,
		Top,
		Bottom,
		Front,
		Back,
	};
	TextureCube(const TextureDescription& desc);

	void Init(const TextureDescription& desc, const void* data) override;
	void SetTextureParameter(const TextureDescription& desc) const override;

	virtual void SetData(const void* data, uint32 size);
	virtual void SetFaceData(uint32 srcName, uint32 srcTarget, int32 srcLevel, int32 srcX, int32 srcY, int32 srcZ, CubeFace face);

	const TextureDescription& GetDesc();

private:
	TextureDescription m_Desc;
};

struct DefaultTextures
{
	Ref<Texture2D> White;  // 用于 Albedo, AO
	Ref<Texture2D> Black;  // 用于 Metallic
	Ref<Texture2D> Normal; // 用于 Normal (128, 128, 255)
	Ref<Texture2D> Gray;   // 用于 Roughness (0.5)
};

template <typename T>
void CreateResource(Ref<T>& tex, const TextureDescription& desc)
{
	if (!tex || tex->GetDesc() != desc)
		tex = CreateRef<T>(desc);
	else
	{
		tex->SetTextureParameter(desc);
	}
}

extern DefaultTextures GDefaultTextures;