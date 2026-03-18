#pragma once
#include "Actor.h"
#include "Common/Core.h"

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
	Ref<ERPPass>	  m_ERPPass;
};
