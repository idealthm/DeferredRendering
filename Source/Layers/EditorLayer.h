#pragma once
#include "Layer.h"
#include "Renderer.h"


class OutlinePanel;
class PropertyPanel;
class ScenePanel;
class CameraController;
class Camera;
class Scene;

enum class EEditorMode
{
	Editor,
	Player,
};

class EditorLayer : public Layer
{
public:
	EditorLayer();
	~EditorLayer() = default;

	void LoadScene(std::string path);

	virtual void OnAttach();
	virtual void OnDetach();
	virtual void OnUpdate(Timestep ts);
	virtual void OnImGuiRender();
	virtual void OnEvent(Event& event);

	void ToggleEditorMode();

private:
	Ref<ScenePanel>			m_ScenePanel;
	Ref<PropertyPanel>		m_PropertyPanel;
	Ref<OutlinePanel>		m_OutlinePanel;

	EEditorMode				m_EditorMode = EEditorMode::Editor;
	uint32_t					m_MainDockingSpace;
	std::string				m_ScenePath;
	Ref<Scene>				m_ActiveScene;

	Ref<Camera>				m_EditorCamera;
	Ref<CameraController>	m_CameraController;
};
