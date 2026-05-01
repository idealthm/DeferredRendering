#pragma once
#include "Actor.h"
#include "Component/ActorComponent.h"

struct alignas(16) LightInfo {
	glm::vec3 position;   // w: 影响半径 (Range / Attenuation Radius)
	float range;

	glm::vec3 color;     // w: 强度 (Intensity)
	float intensity;

	glm::vec3 direction; // w: 光源类型 (0: 方向光, 1: 点光源, 2: 聚光灯)
	int32_t type;

	glm::vec4 params;    // x: 聚光灯内角, y: 聚光灯外角, z: 是否产生阴影, w: 预留
}; 

struct alignas(16) LightData
{
	LightInfo lights[16];
	glm::mat4 uLightVP;
	int32_t NumLights;
};

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