#include "Light.h"

#include <glm/detail/type_quat.hpp>
#include <glm/gtc/quaternion.hpp>

glm::vec3 LightComponent::GetDirection() const
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

glm::mat4 DirectionLightComponent::GetViewProjectMatrix(float range) const
{

	// glm::mat4 testView = glm::lookAt(glm::vec3(0, 0, 20), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	// glm::mat4 testProj = glm::ortho(-7.0f, 7.0f, -7.0f, 7.0f, 0.1f, 100.0f);
	// return testProj * testView;

	glm::vec3 dir = GetDirection();
	// 虚拟位置：从原点往回退
	glm::vec3 pos = -dir * (range * 2.0f); 
    
	// 找一个不与 dir 平行的世界向上向量
	glm::vec3 worldUp = glm::vec3(0, 1, 0);
	if (glm::abs(glm::dot(dir, worldUp)) > 0.99f) {
		worldUp = glm::vec3(0, 0, 1); // 如果太阳直射头顶，改用 Z 轴作为参考
	}

	glm::mat4 view = glm::lookAt(pos, pos + dir, worldUp);
    
	return glm::ortho(-range, range, -range, range, 0.1f, 100.0f) * view;
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
	m_Range = 10.f;
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
