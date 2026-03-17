#pragma once

#include <memory>

#include "Common/Core.h"
#include "Common/Math.h"
#include "glm/glm.hpp"

class StaticMesh;
class Shape;
class Shader;

class ActorComponent
{
public:
	virtual ~ActorComponent() = default;
};


class SceneComponent : public ActorComponent
{
public:
	SceneComponent(const glm::vec3& location = glm::vec3(0), const glm::vec3& rotation = glm::vec3(0)
		, const glm::vec3& scale3D = glm::vec3(1));

	void SetLocation(const glm::vec3& value);
	void SetRotation(const glm::vec3& value);
	void SetScale3D(const glm::vec3& value);

	glm::vec3 GetLocation() const { return m_Transform.GetLocation();}
	glm::vec3 GetRotation() const { return m_Transform.GetRotation();}
	glm::vec3 GetScale3D() const { return m_Transform.GetScale();}

	void SetTransform(const Math::Transform& transform) {m_Transform = transform;}
	const Math::Transform& GetTransform() const { return m_Transform; }

	glm::mat4 GetModelMatrix() const;
	void SetModelMatrix(const glm::mat4& matrix) {m_Transform = Math::Transform::FromMatrix(matrix);}

	void AttachToComponent(std::shared_ptr<SceneComponent> Comp);

protected:
	std::weak_ptr<SceneComponent> ParentComponent;

	Math::Transform m_Transform;
};


class StaticMeshComponent : public SceneComponent
{
public:
	virtual ~StaticMeshComponent() = default;

	virtual void SetMesh(const Ref<StaticMesh>& shape);
	virtual Ref<StaticMesh> GetMesh() { return m_Model; }

protected:
	Ref<StaticMesh> m_Model;
};

