#include "FrameBuffer.h"
#include <glad/glad.h>

#include "Model/Texture.h"

uint32 GetGLCompareFunc(ECompareFunc func)
{
	switch (func)
	{
	case ECompareFunc::Never: return GL_NEVER;
	case ECompareFunc::Less: return GL_LESS;
	case ECompareFunc::Equal: return GL_EQUAL;
	case ECompareFunc::LEqual: return GL_LEQUAL;
	case ECompareFunc::Greater: return GL_GREATER;
	case ECompareFunc::NotEqual: return GL_NOTEQUAL;
	case ECompareFunc::GEqual: return GL_GEQUAL;
	case ECompareFunc::Always: return GL_ALWAYS;
		default: return GL_LESS;
	}
}

FrameBuffer::FrameBuffer()
{
	GLCall(glCreateFramebuffers(1, &m_RendererID));
}

FrameBuffer::~FrameBuffer()
{
	GLCall(glDeleteFramebuffers(1, &m_RendererID));
}

void FrameBuffer::Attach(FBAttachmentInfo& info)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

	m_Info = std::move(info);

	if (m_Info.Depth)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_Info.Depth.GetTextureID(), 0);
		if (m_Info.Depth.LoadAction == FBTextureLoadAction::Clear)
		{
			uint32 color = 0xFFFFFF00;
			glClearTexImage(m_Info.Depth.GetTextureID(), 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, &color);
		}
	}

	for (int32 i = 0; i < m_Info.Attachments.size(); i++)
	{
		auto& attachment = m_Info.Attachments[i];
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, attachment.GetTextureID(), 0);

		if (attachment.LoadAction == FBTextureLoadAction::Clear)
		{
			glClearTexImage(attachment.GetTextureID(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		}
	}

	if (m_Info.Attachments.size() >= 1)
	{
		std::vector<GLenum> buffers(m_Info.Attachments.size());
		for (int i = 0; i < m_Info.Attachments.size(); i++)
		{
			buffers[i] = GL_COLOR_ATTACHMENT0 + i;
		}
		glDrawBuffers(buffers.size(), buffers.data());
	}
	else if (m_Info.Attachments.empty())
	{
		// Only depth-pass
		glDrawBuffer(GL_NONE);
	}

	glViewport(0, 0, m_Info.Width, m_Info.Height);

	if (m_Info.DSS.depthTest)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	glDepthMask(m_Info.DSS.depthWrite ? GL_TRUE : GL_FALSE);
	glDepthFunc(GetGLCompareFunc(m_Info.DSS.compareFunc));

	ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");
}

void FrameBuffer::Clear()
{
	m_Info = {};
}

void FrameBuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
}

void FrameBuffer::Unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int FrameBuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
{
	glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
	int pixelData;
	glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
	return pixelData;
}

uint32 FrameBuffer::GetColorAttachmentRendererID(uint32 index)
{
	ASSERT(index < m_Info.Attachments.size());
	return m_Info.Attachments[index].GetTextureID();
}

