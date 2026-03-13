#pragma once

#include <memory>

#include "Common/Core.h"
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

	const glm::vec3& GetLocation() const;
	const glm::vec3& GetRotation() const;
	const glm::vec3& GetScale3D() const;

	glm::mat4 GetModelMatrix() const;

	void AttachToComponent(std::shared_ptr<SceneComponent> Comp);

protected:
	std::weak_ptr<SceneComponent> ParentComponent;

	glm::vec3 m_Location;
	glm::vec3 m_Rotation;
	glm::vec3 m_scale3D;
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

