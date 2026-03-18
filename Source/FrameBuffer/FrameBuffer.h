#pragma once

#include <string>
#include <vector>
#include <common/Core.h>

#include "Model/Texture.h"

enum class ECompareFunc {
	Never, Less, Equal, LEqual, Greater, NotEqual, GEqual, Always
};

uint32 GetGLCompareFunc(ECompareFunc func);

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

struct FBTextureDesc
{
	uint32 Width, Height;
	EFBTextureFormat		TextureFormat;
	FBTextureLoadAction		LoadAction;
	FBTextureStoreAction	StoreAction;

	Ref<Texture2D>*			TargetTexture;
	// TODO: filtering/wrap
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

uint32 GetGLTexTarget(ETextureTarget target);

struct FBTextureAttachment
{
	uint32					RendererID;
	ETextureTarget			Target = ETextureTarget::Texture2D;
	FBTextureLoadAction		LoadAction;
	FBTextureStoreAction	StoreAction;
	uint32					MipLevel;

	FBTextureAttachment()
	{
		RendererID = 0xFFFFFFFF;
		Target = ETextureTarget::Texture2D;
		LoadAction = FBTextureLoadAction::Load;
		StoreAction = FBTextureStoreAction::Store;
		MipLevel = 0;
	}

	FBTextureAttachment(uint32 renderID, FBTextureLoadAction loadAction, FBTextureStoreAction storeAction, uint32 mipLevel = 0)
	{
		RendererID = renderID;
		Target = ETextureTarget::Texture2D;
		LoadAction = loadAction;
		StoreAction = storeAction;
		MipLevel = mipLevel;
	}

	FBTextureAttachment(uint32 renderID, ETextureTarget target, FBTextureLoadAction loadAction, FBTextureStoreAction storeAction, uint32 mipLevel = 0)
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
	uint32 Width = 0, Height = 0;
	uint32 NumSamples;

	DepthStencilState DSS;

	FBTextureAttachment Depth;
	std::vector<FBTextureAttachment> Attachments;
};

struct FramebufferAttachmentSpecification
{
	FramebufferAttachmentSpecification() = default;
	FramebufferAttachmentSpecification(std::initializer_list<FBTextureDesc> attachments)
		: Attachments(attachments) {}

	std::vector<FBTextureDesc> Attachments;
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

	virtual int ReadPixel(uint32 attachmentIndex, int x, int y);

private:
	uint32 m_RendererID = 0;
	FBAttachmentInfo m_Info;
};

