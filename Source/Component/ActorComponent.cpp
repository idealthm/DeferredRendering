#include "ActorComponent.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Model/StaticMesh.h"
#include "Shader/Program.h"

SceneComponent::SceneComponent(const glm::vec3& location, const glm::vec3& rotation, const glm::vec3& scale3D)
	: m_Transform(location, rotation, scale3D)
{
}

void SceneComponent::SetLocation(const glm::vec3& value)
{
	m_Transform.SetLocation(value);
}

void SceneComponent::SetRotation(const glm::vec3& value)
{
	m_Transform.SetRotation(value);
}

void SceneComponent::SetScale3D(const glm::vec3& value)
{
	m_Transform.SetScale(value);
}

glm::mat4 SceneComponent::GetModelMatrix() const
{
	glm::mat4 parentModel = glm::mat4(1.0f);
	if (!ParentComponent.expired())
	{
		parentModel = ParentComponent.lock()->GetModelMatrix();
	}

	return m_Transform.GetModelMatrix() * parentModel;
}

void SceneComponent::AttachToComponent(std::shared_ptr<SceneComponent> Comp)
{
	ParentComponent = Comp;
}

void StaticMeshComponent::SetMesh(const std::shared_ptr<StaticMesh>& shape)
{
	m_Model = shape;
}
