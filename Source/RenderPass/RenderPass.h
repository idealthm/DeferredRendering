#pragma once
#include <memory>
#include <vector>

#include "Common/Core.h"


class Scene;
class Shader;

class RenderPass
{
public:
	RenderPass(uint32 width, uint32 height){}

	virtual ~RenderPass() = default;

	virtual void OnWindowSizeChanged(int32 width, int32 height) {}

	virtual void PrePass(const Ref<Scene>& scene) {}
	virtual void OnPass(const Ref<Scene>& scene) {}
	virtual void PostPass(const Ref<Scene>& scene) {}

private:
	std::vector<std::shared_ptr<Shader>> Shaders;
};
