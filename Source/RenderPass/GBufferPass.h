#pragma once
#include <string>

#include "RenderPass.h"

struct RenderContext;
class FrameBuffer;

class GBufferPass : public RenderPass
{
public:
	GBufferPass(uint32 width, uint32 height);
	virtual ~GBufferPass() = default;

	uint32 GetColorAttachmentRendererID(const std::string& name) const;

	virtual void OnWindowSizeChanged(int32 width, int32 height) override;

	virtual void PrePass(RenderContext& context) override;
	virtual void OnPass(RenderContext& context) override;
	virtual void PostPass(RenderContext& context) override;

private:
	Ref<Shader>			m_GBufferShader;
	Ref<FrameBuffer>	m_GBufferFBO;
};
