#pragma once
#include "Component/ActorComponent.h"

class LightComponent : SceneComponent
{
public:
	
};


class DirectionLight : LightComponent
{
public:

private:
	glm::vec3 m_Direction;
	glm::vec3 m_Ambient;
	glm::vec3 m_Diffuse;
	glm::vec3 m_Specular;
};