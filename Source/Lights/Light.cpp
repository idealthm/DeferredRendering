#include "Light.h"

#include <glm/detail/type_quat.hpp>
#include <glm/gtc/quaternion.hpp>

LightComponent::LightComponent()
{
	m_Ambient = glm::vec3(0.1f);
	m_Diffuse = glm::vec3(1.f);
	m_Specular = glm::vec3(0.3f);
}

DirectionLightComponent::DirectionLightComponent()
{
}

glm::vec3 DirectionLightComponent::GetDirection() const
{
	// default to Z
	glm::vec3 radians = glm::radians(m_Rotation);
    
	// 创建四元数 (Yaw -> Pitch -> Roll)
	glm::quat rotation = 
		glm::angleAxis(radians.y, glm::vec3(0, 1, 0)) * // Yaw
		glm::angleAxis(radians.x, glm::vec3(1, 0, 0)) * // Pitch
		glm::angleAxis(radians.z, glm::vec3(0, 0, 1)); // Roll
    
	return glm::normalize(rotation * glm::vec3(0.0f, 0.0f, -1.0f));
}

glm::vec3 DirectionLightComponent::GetUPDirection() const
{
	glm::vec3 radians = glm::radians(m_Rotation);
    
	// 创建四元数 (Yaw -> Pitch -> Roll)
	glm::quat rotation = 
		glm::angleAxis(radians.y, glm::vec3(0, 1, 0)) * // Yaw
		glm::angleAxis(radians.x, glm::vec3(1, 0, 0)) * // Pitch
		glm::angleAxis(radians.z, glm::vec3(0, 0, 1)); // Roll
	return glm::normalize(rotation * glm::vec3(0.0f, 1.0f, 0.0f));
}

PointLightComponent::PointLightComponent()
{
	m_Constant = 1.f;
	m_Linear = 0.014f;
	m_Quadratic = 0.0007f;
}

SpotLightComponent::SpotLightComponent()
{
	m_Cutoff = 30.f;
	m_OuterCutoff = 45.f;
}

glm::vec3 SpotLightComponent::GetDirection() const
{
	return glm::normalize(glm::mat3_cast(glm::quat(m_Rotation)) * glm::vec3(0.f, 0.f, 1.f));
}

DirectionLightActor::DirectionLightActor()
{
	m_DirectionLightComponent = AddComponent<DirectionLightComponent>();
	SetRootComponent(m_DirectionLightComponent);
}

PointLightActor::PointLightActor()
{
	m_PointLightComponent = AddComponent<PointLightComponent>();
	SetRootComponent(m_PointLightComponent);
}

SpotLightActor::SpotLightActor()
{
	m_SpotLightComponent = AddComponent<SpotLightComponent>();
	SetRootComponent(m_SpotLightComponent);
}
