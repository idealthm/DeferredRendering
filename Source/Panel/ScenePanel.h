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

	uint32 GetGizmoMode() const;
	void ToggleGizmoMode();
private:
	uint32	m_GizmoMode = 0;
};
