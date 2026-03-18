#pragma once
#include <string>

#include "RenderPass.h"

class TextureCube;
class Texture2D;

class ERPPass : public RenderPass
{
public:
	ERPPass(const std::string& hdrFilePath, uint32 size);
	virtual ~ERPPass();

	uint32 GetRenderTimes() override { return 6;}
	void Setup(FBAttachmentInfo& info, uint32 step) override;
	void Execute(Ref<Scene> scene, uint32 step) override;

private:
	Ref<Shader>			m_Shader;

	Ref<Texture2D>		m_HDRMap;
	Ref<TextureCube>	m_CubeMapTexture;
};
