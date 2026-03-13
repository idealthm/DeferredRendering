#pragma once
#include <memory>
#include <vector>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>

#include "common/Core.h"

class StaticMesh;
class StaticMeshComponent;
class SceneComponent;
class ActorComponent;

class Actor
{
public:
	Actor();

	virtual void OnSpawn();

	void ValidateRootComponent();

	Ref<SceneComponent> GetRootComponent() const;

	glm::mat4 GetModelMatrix() const;

	void SetRootComponent(const Ref<SceneComponent>& component);

	void SetPosition(const glm::vec3& position) const;
	void SetRotation(const glm::vec3& rotation) const;
	void SetScale3D(const glm::vec3& scale) const;

	template<typename T, typename... Args, typename = std::enable_if_t<std::is_base_of_v<ActorComponent, T>>>
	Ref<T> AddComponent(Args... args)
	{
		auto ComponentRef =  CreateRef<T>(std::forward<Args>(args)...);
		Components.push_back(ComponentRef);
		if (!RootComponent.expired())
		{
			ComponentRef->AttachToComponent(RootComponent.lock());
		}
		return ComponentRef;
	}

	template<typename T, typename = std::enable_if_t<std::is_base_of_v<ActorComponent, T>>>
	Ref<T> GetComponent()
	{
		for (const auto& component : Components)
		{
			if (auto TComp = std::dynamic_pointer_cast<T>(component))
			{
				return TComp;
			}
		}
		return {};
	}

	const std::vector<Ref<ActorComponent>>& GetComponents() const;
protected:
	std::weak_ptr<SceneComponent>		RootComponent;

	std::vector<Ref<ActorComponent>>	Components;
};


class StaticMeshActor : public Actor
{
public:
	StaticMeshActor();

	void SetStaticMesh(const Ref<StaticMesh>& staticMesh) const;

protected:
	Ref<StaticMeshComponent> m_StaticMeshComp;
};