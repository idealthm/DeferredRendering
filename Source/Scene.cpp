#include "Scene.h"

#include "Camera/Camera.h"
#include "Renderer.h"
#include "FrameBuffer/FrameBuffer.h"
#include "Lights/Light.h"


Scene::Scene(uint32 width, uint32 height, const Ref<Camera>& camera)
	: m_Camera(camera), m_Width(width), m_Height(height)
{
	m_RenderContext.FrameBuffer = CreateScope<FrameBuffer>();
	m_RenderContext.FrameDataUB = CreateScope<ParamBuffer<FrameData>>(0);
	m_RenderContext.LightDataUB = CreateScope<ParamBuffer<LightData>>(1);
	m_RenderContext.ShadowWidth = 2048.f * 1;
	m_RenderContext.ShadowHeight = 2048.f * 1;
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