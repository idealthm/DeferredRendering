#pragma once
#include <memory>
#include <set>
#include <vector>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>

#include "Renderer.h"
#include "Common/Core.h"

class Camera;
class Actor;

class Scene
{
public:
	friend class Window;
	Scene(uint32 width, uint32 height, const Ref<Camera>& camera);

	template<typename T, std::enable_if_t<std::is_base_of<Actor, T>::value, int> = 0>
	Ref<T> SpawnActor(const glm::vec3& position = glm::vec3(0), const glm::vec3& rotation = glm::vec3(0)
		, const glm::vec3& scale = glm::vec3(1))
	{
		Ref<T> actor = std::make_shared<T>();
		actor->ValidateRootComponent();
		actor->SetPosition(position);
		actor->SetRotation(rotation);
		actor->SetScale3D(scale);
		Actors.insert(actor);
		return actor;
	}

	const std::set<Ref<Actor>>& GetActors() const;

	glm::vec3 GetCameraPosition() const;
	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetProjectionMatrix() const;

	uint32 GetWidth() const {return m_Width;}
	void SetWidth(uint32 width) {m_Width = width;}
	uint32 GetHeight() const {return m_Height;}
	void SetHeight(uint32 height) {m_Height = height;}

	RenderContext& GetRenderContext() {return m_RenderContext;}
	const RenderContext& GetRenderContext() const {return m_RenderContext;}
	
private:
	Ref<Camera> m_Camera;

	uint32 m_Width, m_Height;

	std::set<Ref<Actor>>	Actors;

	RenderContext	m_RenderContext;
};
