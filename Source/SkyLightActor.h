#pragma once
#include "Actor.h"
#include "Common/Core.h"

class CubeMapConvolution;
class ERPPass;
class TextureCube;
class SkyComponent;

class SkyLightActor : public Actor
{
public:
	SkyLightActor();
	~SkyLightActor() = default;

	void OnSpawn() override;
private:
	Ref<SkyComponent> m_SkyComponent;
};
