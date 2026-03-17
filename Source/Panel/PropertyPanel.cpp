#include "PropertyPanel.h"

#include "ImGui/imgui.h"

PropertyPanel::PropertyPanel(const std::string& panelName)
	: EditorPanel(panelName)
{
}

void PropertyPanel::OnImGuiRender(Ref<Scene>& scene)
{
	ImGui::Begin(m_PanelName.c_str());
	m_Focused = ImGui::IsWindowFocused();
	m_Hovered = ImGui::IsWindowHovered();
	ImGui::End();
}
