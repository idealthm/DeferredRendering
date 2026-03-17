#pragma once
#include "Common/Core.h"

class Event;
enum class EEditorMode;
class Camera;

class CameraController
{
public:
	CameraController();

	void OnEvent(Event& event) const;
	void OnUpdate(float deltaTime) const;

	void ProcessCamera(Ref<Camera> camera);

private:
	Ref<Camera>		m_ActiveCamera;
};
