#include "Actor.h"

#include "Renderer.h"
#include "Component/ActorComponent.h"

Actor::Actor()
{
}

void Actor::ValidateRootComponent()
{
	if (RootComponent.expired())
	{
		auto Root = std::make_shared<SceneComponent>();
		Components.push_back(Root);
		SetRootComponent(Root);
	}
}

Ref<SceneComponent> Actor::GetRootComponent() const
{
	return RootComponent.lock();
}

glm::mat4 Actor::GetModelMatrix() const
{
	return RootComponent.lock()->GetModelMatrix();
}

void Actor::SetRootComponent(const Ref<SceneComponent>& component)
{
	RootComponent = component;
}

void Actor::SetPosition(const glm::vec3& position) const
{
	RootComponent.lock()->SetLocation(position);
}

void Actor::SetRotation(const glm::vec3& rotation) const
{
	RootComponent.lock()->SetRotation(rotation);
}

void Actor::SetScale3D(const glm::vec3& scale) const
{
	RootComponent.lock()->SetScale3D(scale);
}

const std::vector<Ref<ActorComponent>>& Actor::GetComponents() const
{
	return Components;
}

StaticMeshActor::StaticMeshActor()
{
	m_StaticMeshComp = std::make_shared<StaticMeshComponent>();
	Components.push_back(m_StaticMeshComp);
	SetRootComponent(m_StaticMeshComp);
}

void StaticMeshActor::SetStaticMesh(const Ref<StaticMesh>& staticMesh) const
{
	m_StaticMeshComp->SetMesh(staticMesh);
}
