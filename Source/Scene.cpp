#include "Scene.h"

#include "Camera/Camera.h"


Scene::Scene(uint32 width, uint32 height, const Ref<Camera>& camera)
	: m_Camera(camera), m_Width(width), m_Height(height)
{
}

const std::set<std::shared_ptr<Actor>>& Scene::GetActors() const
{
	return Actors;
}

glm::vec3 Scene::GetCameraPosition() const
{
	return m_Camera->GetPosition();
}

glm::mat4 Scene::GetViewMatrix() const
{
	return m_Camera->GetViewMatrix();
}

glm::mat4 Scene::GetProjectionMatrix() const
{
	return m_Camera->GetProjectionMatrix(1.f * m_Width / m_Height);
}

uint32 Scene::GetWidth() const
{
	return m_Width;
}

uint32 Scene::GetHeight() const
{
	return m_Height;
}
