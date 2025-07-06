#pragma once
#include <memory>
#include <set>
#include <vector>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>

#include "Common/Core.h"

class Camera;
class Actor;

class Scene
{
public:
	friend class WindowsWindow;
	Scene(uint32 width, uint32 height);

	template<typename T, std::enable_if_t<std::is_base_of<Actor, T>::value, int> = 0>
	std::shared_ptr<T> SpawnActor(const glm::vec3& position = glm::vec3(0), const glm::vec3& rotation = glm::vec3(0)
		, const glm::vec3& scale = glm::vec3(1))
	{
		std::shared_ptr<T> actor = std::make_shared<T>();
		actor->ValidateRootComponent();
		actor->SetPosition(position);
		actor->SetRotation(rotation);
		actor->SetScale3D(scale);
		Actors.insert(actor);
		return actor;
	}

	const std::set<std::shared_ptr<Actor>>& GetActors() const;

	void ProcessInput(struct GLFWwindow* window, float deltaTime) const;
	void ScrollCallback(double xoffset, double yoffset);
	void MouseCallback(double xPos, double yPos);

	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetProjectionMatrix() const;

	uint32 GetWidth() const;
	uint32 GetHeight() const;
	
private:
	uint32 m_Width, m_Height;

	std::shared_ptr<Camera>				m_Camera;

	std::set<std::shared_ptr<Actor>>	Actors;
};
