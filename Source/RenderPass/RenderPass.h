#pragma once
#include <memory>
#include <vector>

#include "Common/Core.h"


class Scene;
class Shader;

class RenderPass
{
public:
	virtual ~RenderPass() = default;

	virtual bool Init(std::shared_ptr<Scene>& scene);

	virtual void PrePass(std::shared_ptr<Scene>& scene);

	virtual void OnPass(std::shared_ptr<Scene>& scene);

	virtual void PostPass(std::shared_ptr<Scene>& scene);

private:
	std::vector<std::shared_ptr<Shader>> Shaders;
};
