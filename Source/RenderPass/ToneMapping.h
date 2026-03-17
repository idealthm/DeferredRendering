#pragma once
#include "RenderPass.h"

class ToneMapping : public RenderPass
{
public:
	ToneMapping();

	void Setup(FBAttachmentInfo& info) override;
	void Execute(Ref<Scene> scene) override;

private:
	Ref<Shader> m_Shader;
};
