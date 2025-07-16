#pragma once
#include <memory>
#include <vector>

#include "Common/Core.h"


struct RenderContext;
class Scene;
class Shader;

class RenderPass
{
public:
	RenderPass(uint32 width, uint32 height){}

	virtual ~RenderPass() = default;

	virtual void OnWindowSizeChanged(int32 width, int32 height) {}

	virtual void PrePass(RenderContext& scene) {}
	virtual void OnPass(RenderContext& scene) {}
	virtual void PostPass(RenderContext& scene) {}

private:
	std::vector<std::shared_ptr<Shader>> Shaders;
};
