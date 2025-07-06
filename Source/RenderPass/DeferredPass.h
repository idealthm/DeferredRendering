#pragma once
#include "RenderPass.h"

class DeferredPass : public RenderPass
{
public:
	~DeferredPass();

	virtual bool Init(std::shared_ptr<Scene>& scene) override;

	virtual void PrePass(std::shared_ptr<Scene>& scene);

	virtual void OnPass(std::shared_ptr<Scene>& scene);

	virtual void PostPass(std::shared_ptr<Scene>& scene);

private:
	uint32 gBuffer = 0;
	uint32 gPosition = 0;
	uint32 gNormal = 0;
	uint32 gAlbedo = 0;
	uint32 rboDepth = 0;

	std::unique_ptr<Shader> GeometryShader;
	std::unique_ptr<Shader> LightShader;
};
