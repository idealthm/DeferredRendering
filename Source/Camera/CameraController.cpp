#include "CameraController.h"

#include "Camera.h"
#include "Events/Event.h"
#include "Layers/EditorLayer.h"

CameraController::CameraController()
{
}

void CameraController::OnEvent(Event& event) const
{
	if (m_ActiveCamera)
	{
		m_ActiveCamera->OnEvent(event);
	}
}

void CameraController::OnUpdate(float deltaTime) const
{
	if (m_ActiveCamera)
	{
		m_ActiveCamera->OnUpdate(deltaTime);
	}
}

void CameraController::ProcessCamera(Ref<Camera> camera)
{
	m_ActiveCamera = camera;
}

