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
	void Setup(RenderContext& ctx) override;
	void Execute(Ref<Scene> scene, RenderContext& ctx) override;

private:
	Ref<Program>			m_Shader;
	Ref<Texture>		m_HDRMap;
};
