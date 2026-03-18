#pragma once
#include "RenderPass.h"

class ToneMapping : public RenderPass
{
public:
	ToneMapping();

	void Setup(FBAttachmentInfo& info, uint32 step) override;
	void Execute(Ref<Scene> scene, uint32 step) override;

private:
	Ref<Shader> m_Shader;
};
