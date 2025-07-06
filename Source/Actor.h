#pragma once
#include <memory>
#include <vector>
#include <glm/vec3.hpp>

class StaticMeshComponent;
class SceneComponent;
class ActorComponent;

class Actor
{
public:
	Actor();

	void ValidateRootComponent();

	std::shared_ptr<SceneComponent> GetRootComponent() const;

	void SetRootComponent(const std::shared_ptr<SceneComponent>& component);

	void SetPosition(const glm::vec3& position) const;
	void SetRotation(const glm::vec3& rotation) const;
	void SetScale3D(const glm::vec3& scale) const;

	const std::vector<std::shared_ptr<ActorComponent>>& GetComponents() const;
protected:
	std::weak_ptr<SceneComponent>					RootComponent;

	std::vector<std::shared_ptr<ActorComponent>>	Components;
};


class StaticMeshActor : public Actor
{
public:
	StaticMeshActor();

protected:
	std::shared_ptr<StaticMeshComponent> m_StaticMeshComp;
};

class CubeActor : public StaticMeshActor
{
public:
	CubeActor();
};


class QuadActor : public StaticMeshActor
{
public:
	QuadActor();
};