#pragma once
#include "RenderPass.h"

class ToneMapping : public RenderPass
{
public:
	ToneMapping(uint32 width, uint32 height);

	void Setup(RenderContext& ctx, FBAttachmentInfo& info) override;
	void Execute(Ref<Scene> scene) override;

private:
	Ref<Shader> m_Shader;
};
