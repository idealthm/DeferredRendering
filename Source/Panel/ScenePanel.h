#pragma once
#include <string>

#include "EditorPanel.h"
#include "Common/Core.h"

class Scene;

class ScenePanel : public EditorPanel
{
public:
	ScenePanel(const std::string& panelName);

	void OnImGuiRender(Ref<Scene>& scene) override;

	uint32_t GetGizmoMode() const;
	void ToggleGizmoMode();
private:
	uint32_t	m_GizmoMode = 0;
};
