#include "Actor.h"

#include "Renderer.h"
#include "Component/ActorComponent.h"
#include "Model/Shape.h"

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

std::shared_ptr<SceneComponent> Actor::GetRootComponent() const
{
	return RootComponent.lock();
}

void Actor::SetRootComponent(const std::shared_ptr<SceneComponent>& component)
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

const std::vector<std::shared_ptr<ActorComponent>>& Actor::GetComponents() const
{
	return Components;
}

StaticMeshActor::StaticMeshActor()
{
	m_StaticMeshComp = std::make_shared<StaticMeshComponent>();
	Components.push_back(m_StaticMeshComp);
	SetRootComponent(m_StaticMeshComp);
}

CubeActor::CubeActor()
{
	m_StaticMeshComp->SetMesh(std::make_shared<Cube>());
}

QuadActor::QuadActor()
{
	m_StaticMeshComp->SetMesh(std::make_shared<Quad>());
}
