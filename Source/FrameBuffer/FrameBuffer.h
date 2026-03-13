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

struct FBTextureAttachment
{
	Ref<Texture2D>*			Texture;
	TextureDescription		Desc;
	FBTextureLoadAction		LoadAction;
	FBTextureStoreAction	StoreAction;

	FBTextureAttachment()
	{
		Texture = nullptr;
		LoadAction = FBTextureLoadAction::Load;
		StoreAction = FBTextureStoreAction::Store;
	}

	FBTextureAttachment(Ref<Texture2D>* texture, const TextureDescription& desc, FBTextureLoadAction loadAction, FBTextureStoreAction storeAction)
	{
		Texture = texture;
		Desc = desc;
		LoadAction = loadAction;
		StoreAction = storeAction;
	}

	FBTextureAttachment(const FBTextureAttachment& other)
	{
		Texture = other.Texture;
		Desc = other.Desc;
		LoadAction = other.LoadAction;
		StoreAction = other.StoreAction;
	}

	operator bool () const {return Texture != nullptr;}
	Ref<Texture2D> GetTexture() const {return *Texture;}
	uint32 GetTextureID() const {return GetTexture() ? GetTexture()->GetRendererID() : 0;}
};

struct FBAttachmentInfo
{
	uint32 Width, Height;
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

	virtual uint32 GetDepthRendererID() { return m_Info.Depth.Texture ? m_Info.Depth.GetTextureID() : 0; }

	virtual uint32 GetColorAttachmentRendererID(uint32 index);

private:
	uint32 m_RendererID = 0;
	FBAttachmentInfo m_Info;
};

