#pragma once
#include "EditorPanel.h"
#include "Common/Core.h"

class Scene;

class OutlinePanel : public EditorPanel
{
public:
	OutlinePanel(const std::string& panelName);

	virtual void OnImGuiRender(Ref<Scene>& scene) override;
};
