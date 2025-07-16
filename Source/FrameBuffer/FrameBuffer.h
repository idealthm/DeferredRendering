#pragma once

#include <string>
#include <vector>
#include <common/Core.h>

enum class FramebufferTextureFormat
{
	None = 0,

	// Color
	RGBA8,
	RED_INTEGER,

	RGBA16F,

	// Depth/stencil
	DEPTH24STENCIL8,

	// Defaults
	Depth = DEPTH24STENCIL8
};

struct FramebufferTextureSpecification
{
	FramebufferTextureSpecification() = default;
	FramebufferTextureSpecification(const std::string& name, FramebufferTextureFormat format)
		: BufferName(name),TextureFormat(format) {}

	std::string BufferName;
	FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
	// TODO: filtering/wrap
};

struct FramebufferAttachmentSpecification
{
	FramebufferAttachmentSpecification() = default;
	FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments)
		: Attachments(attachments) {}

	std::vector<FramebufferTextureSpecification> Attachments;
};

struct FramebufferSpecification
{
	uint32 Width = 0, Height = 0;
	FramebufferAttachmentSpecification Attachments;
	uint32 Samples = 1;

	bool SwapChainTarget = false;
};


class FrameBuffer
{
public:
	FrameBuffer(const FramebufferSpecification& spec);
	virtual ~FrameBuffer();

	void Invalidate();

	virtual void Bind();
	virtual void Unbind();

	virtual void Resize(uint32 width, uint32 height) ;
	virtual int ReadPixel(uint32 attachmentIndex, int x, int y);

	virtual void ClearAttachment(uint32 attachmentIndex, int value);

	virtual uint32 GetColorAttachmentRendererID(const std::string& name) const;
	virtual uint32 GetColorAttachmentRendererID(uint32 index = 0) const { ASSERT(index < m_ColorAttachments.size()); return m_ColorAttachments[index]; }

	virtual const FramebufferSpecification& GetSpecification() const { return m_Specification; }
private:
	uint32 m_RendererID = 0;
	FramebufferSpecification m_Specification;

	std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;
	FramebufferTextureSpecification m_DepthAttachmentSpecification;

	std::vector<uint32> m_ColorAttachments;
	uint32 m_DepthAttachment = 0;
};

