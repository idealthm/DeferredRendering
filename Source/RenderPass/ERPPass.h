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

	void Setup(FBAttachmentInfo& info) override;
	void Execute(Ref<Scene> scene) override;

	Ref<TextureCube> GetCubeMapTexture() const { return m_CubeMapTexture; }

private:
	Ref<Shader>			m_Shader;

	Ref<Texture2D>		m_HDRMap;
	Ref<TextureCube>	m_CubeMapTexture;
};
