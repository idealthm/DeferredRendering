#pragma once
#include <string>

#include "RenderPass.h"
#include "Shapes/UnitCube.h"

class Texture;

class ERPPass : public RenderPass
{
public:
	ERPPass(const std::string& hdrFilePath, uint32_t size);
	virtual ~ERPPass();

	uint32_t GetRenderTimes() override { return 6;}
	void Setup(FBAttachmentInfo& info, uint32_t step, RenderContext& ctx) override;
	void Execute(Ref<Scene> scene, uint32_t step, RenderContext& ctx) override;

private:
	Ref<Program>			m_Shader;
	UnitCube			m_UnitCube;
	Ref<Texture>		m_HDRMap;
};
