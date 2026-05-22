#pragma once
#include "Actor.h"
#include "Component/ActorComponent.h"

class LightComponent : public SceneComponent
{
public:
	LightComponent() = default;

	glm::vec3 GetColor() const {return m_Color;}
	float GetIntensity() const {return m_Intensity;}
	glm::vec3 GetDirection() const;
public:
	float m_Intensity = 1.0f;
	glm::vec3 m_Color = glm::vec3(1.0f, 1.0f, 1.0f);
};


class DirectionLightComponent : public LightComponent
{
public:
	DirectionLightComponent() = default;

	glm::mat4 GetViewProjectMatrix(float range) const;

	glm::vec3 GetUPDirection() const;
};

class PointLightComponent : public LightComponent
{
public:
	PointLightComponent();

public:
	float m_Range;
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