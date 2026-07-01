#pragma once
#include "ActorComponent.h"
#include "Common/Core.h"
#include "Model/Texture.h"


class SkyComponent : public SceneComponent
{
public:
	void SetCubeMapTexture(Ref<Texture> cubeMapTexture);
};
