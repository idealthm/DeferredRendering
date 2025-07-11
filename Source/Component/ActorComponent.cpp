#include "ActorComponent.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>

#include "Model/StaticMesh.h"
#include "Shader/Shader.h"

SceneComponent::SceneComponent(const glm::vec3& location, const glm::vec3& rotation, const glm::vec3& scale3D)
	: m_Location(location), m_Rotation(rotation), m_scale3D(scale3D)
{
}

void SceneComponent::SetLocation(const glm::vec3& value)
{
	m_Location = value;
}

void SceneComponent::SetRotation(const glm::vec3& value)
{
	m_Rotation = value;
}

void SceneComponent::SetScale3D(const glm::vec3& value)
{
	m_scale3D = value;
}

const glm::vec3& SceneComponent::GetLocation() const
{
	return m_Location;
}

const glm::vec3& SceneComponent::GetRotation() const
{
	return m_Rotation;
}

const glm::vec3& SceneComponent::GetScale3D() const
{
	return m_scale3D;
}

glm::mat4 SceneComponent::GetModelMatrix() const
{
	glm::mat4 parentModel = glm::mat4(1.0f);
	if (!ParentComponent.expired())
	{
		parentModel = ParentComponent.lock()->GetModelMatrix();
	}

	glm::quat quat = glm::angleAxis(glm::radians(m_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)) *  // Yaw (Y)
				 glm::angleAxis(glm::radians(m_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f)) *  // Pitch (X)
				 glm::angleAxis(glm::radians(m_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));   // Roll (Z)

	return glm::translate(glm::mat4(1.0), m_Location) * glm::mat4_cast(quat) * glm::scale(glm::mat4(1.0), m_scale3D) * parentModel;

	// return glm::translate(glm::mat4(1.0), m_Location) * glm::mat4_cast(glm::quat(m_Rotation)) * glm::scale(glm::mat4(1.0), m_scale3D) * parentModel;
}

void SceneComponent::Draw(Shader& shader)
{
	// Do nothing.
}

void SceneComponent::AttachToComponent(std::shared_ptr<SceneComponent> Comp)
{
	ParentComponent = Comp;
}

void StaticMeshComponent::SetMesh(const std::shared_ptr<StaticMesh>& shape)
{
	m_Model = shape;
}

void StaticMeshComponent::Draw(Shader& shader)
{
	m_Model->Draw(shader);
}
