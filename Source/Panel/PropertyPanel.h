#pragma once
#include "EditorPanel.h"
#include "Common/Core.h"

class Scene;

class PropertyPanel : public EditorPanel
{
public:
	PropertyPanel(const std::string& panelName);

	void OnImGuiRender(Ref<Scene>& scene) override;
};
