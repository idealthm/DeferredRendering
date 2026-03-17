#pragma once
#include <string>
#include <utility>

#include "Scene.h"

class EditorPanel
{
public:
	EditorPanel(std::string panelName)
		: m_PanelName(std::move(panelName))
	{}

	virtual void OnImGuiRender(Ref<Scene>& scene) {} 

	bool IsPanelHovered() const {return m_Hovered;}
	bool IsPanelFocused() const {return m_Focused;}
protected:
	bool m_Hovered = false;
	bool m_Focused = false;
	std::string m_PanelName;
};
