#pragma once
#include <memory>
#include <set>
#include <vector>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>

#include "Renderer.h"
#include "Common/Core.h"
#include "Common/Math.h"

class Camera;
class Actor;

class Scene : public std::enable_shared_from_this<Scene>
{
public:
	friend class Window;
	Scene();

	template<typename T, std::enable_if_t<std::is_base_of<Actor, T>::value, int> = 0>
	Ref<T> SpawnActor(const glm::vec3& location = glm::vec3(0), const glm::vec3& rotation = glm::vec3(0)
		, const glm::vec3& scale = glm::vec3(1))
	{
		Ref<T> actor = std::make_shared<T>();
		actor->ValidateRootComponent();
		actor->SetTransform(Math::Transform(location, rotation, scale));
		actor->m_WeakScene = weak_from_this();
		Actors.insert(actor);
		actor->OnSpawn();
		return actor;
	}

	const std::set<Ref<Actor>>& GetActors() const;

	template<typename ActorClass>
	Ref<ActorClass> GetActor() const
	{
		for (auto actor : Actors)
		{
			if (Ref<ActorClass> target = std::dynamic_pointer_cast<ActorClass>(actor))
				return target;
		}
		return nullptr;
	}

	RenderContext& GetRenderContext() {return m_RenderContext;}
	const RenderContext& GetRenderContext() const {return m_RenderContext;}
	RenderContext	m_RenderContext;
	
private:
	std::set<Ref<Actor>>	Actors;

};
