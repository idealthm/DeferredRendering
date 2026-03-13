#pragma once
#include "ActorComponent.h"
#include "Common/Core.h"

class TextureCube;

class SkyComponent : public SceneComponent
{
public:
	void SetCubeMapTexture(Ref<TextureCube> cubeMapTexture);
	Ref<TextureCube> GetCubeMapTexture() {return m_CubeMap;}
private:
	Ref<TextureCube> m_CubeMap;
};
