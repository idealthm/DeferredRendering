#pragma once
#include <string>

#include "RenderPass.h"
#include "Shapes/UnitCube.h"

class RenderTarget;
class MaterialInstance;
class Texture;

class ERPPass
{
public:
	virtual ~ERPPass() = default;

	void Render(Ref<Texture> texture, Ref<Texture>& CubeMap);

	void Setup(RenderContext& ctx);
	void Execute(Ref<Scene> scene, RenderContext& ctx);

private:
};
