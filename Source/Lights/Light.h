#pragma once
#include "Actor.h"
#include "Component/ActorComponent.h"

class LightComponent : public SceneComponent
{
public:
	LightComponent();

public:
	glm::vec3 m_Ambient;
	glm::vec3 m_Diffuse;
	glm::vec3 m_Specular;
};


class DirectionLightComponent : public LightComponent
{
public:
	DirectionLightComponent();

	glm::vec3 GetDirection() const;
	glm::vec3 GetUPDirection() const;
};

class PointLightComponent : public LightComponent
{
public:
	PointLightComponent();

public:
	float m_Constant;
	float m_Linear;
	float m_Quadratic;
};

class SpotLightComponent : public PointLightComponent
{
public:
	SpotLightComponent();

	glm::vec3 GetDirection() const;
public:
	float m_Cutoff;
	float m_OuterCutoff;
};


class DirectionLightActor : public Actor
{
public:
	DirectionLightActor();
private:
	Ref<DirectionLightComponent> m_DirectionLightComponent;
};


class PointLightActor : public Actor
{
public:
	PointLightActor();
private:
	Ref<PointLightComponent> m_PointLightComponent;
};

class SpotLightActor : public Actor
{
public:
	SpotLightActor();
private:
	Ref<SpotLightComponent> m_SpotLightComponent;
};