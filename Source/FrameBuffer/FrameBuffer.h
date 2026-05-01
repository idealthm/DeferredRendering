#pragma once

#include <string>
#include <vector>
#include <common/Core.h>

#include "Model/Texture.h"

enum class ECompareFunc {
	Never, Less, Equal, LEqual, Greater, NotEqual, GEqual, Always
};

uint32_t GetGLCompareFunc(ECompareFunc func);

struct DepthStencilState {
	bool depthTest = true;
	bool depthWrite = true;
	ECompareFunc compareFunc = ECompareFunc::Less;
};

enum class EFBTextureFormat
{
	None = 0,

	// Color
	RGBA8,
	RED_INTEGER,

	RG16,
	RGBA16F,

	// Depth/stencil
	DEPTH24STENCIL8,

	// Defaults
	Depth = DEPTH24STENCIL8
};

enum class FBTextureLoadAction
{
	Load,
	Clear,
};


enum class FBTextureStoreAction
{
	Store,
	Discard,
};

enum class FBAttachmentType
{
	Color,
	Depth,
};

enum class ETextureTarget
{
	// Texture2D
	Texture2D,

	// CubeMap
	Positive_X,
	Negative_X,
	Positive_Y,
	Negative_Y,
	Positive_Z,
	Negative_Z,

	//
};

uint32_t GetGLTexTarget(ETextureTarget target);

struct FBTextureAttachment
{
	uint32_t					RendererID;
	ETextureTarget			Target = ETextureTarget::Texture2D;
	FBTextureLoadAction		LoadAction;
	FBTextureStoreAction	StoreAction;
	uint32_t					MipLevel;

	FBTextureAttachment()
	{
		RendererID = 0xFFFFFFFF;
		Target = ETextureTarget::Texture2D;
		LoadAction = FBTextureLoadAction::Load;
		StoreAction = FBTextureStoreAction::Store;
		MipLevel = 0;
	}

	FBTextureAttachment(uint32_t renderID, FBTextureLoadAction loadAction, FBTextureStoreAction storeAction, uint32_t mipLevel = 0)
	{
		RendererID = renderID;
		Target = ETextureTarget::Texture2D;
		LoadAction = loadAction;
		StoreAction = storeAction;
		MipLevel = mipLevel;
	}

	FBTextureAttachment(uint32_t renderID, ETextureTarget target, FBTextureLoadAction loadAction, FBTextureStoreAction storeAction, uint32_t mipLevel = 0)
	{
		RendererID = renderID;
		Target = target;
		LoadAction = loadAction;
		StoreAction = storeAction;
		MipLevel = mipLevel;
	}

	FBTextureAttachment(const FBTextureAttachment& other)
	{
		RendererID = other.RendererID;
		Target = other.Target;
		LoadAction = other.LoadAction;
		StoreAction = other.StoreAction;
		MipLevel = other.MipLevel;
	}

	operator bool () const {return RendererID != 0xFFFFFFFF;}
};

struct FBAttachmentInfo
{
	uint32_t Width = 0, Height = 0;
	uint32_t NumSamples;

	DepthStencilState DSS;

	FBTextureAttachment Depth;
	std::vector<FBTextureAttachment> Attachments;
};

class FrameBuffer
{
public:
	FrameBuffer();
	virtual ~FrameBuffer();

	void Attach(FBAttachmentInfo& info);

	void Clear();

	virtual void Bind();
	virtual void Unbind();

	virtual int ReadPixel(uint32_t attachmentIndex, int x, int y);

private:
	uint32_t m_RendererID = 0;
	FBAttachmentInfo m_Info;
};

