#include "OutlinePanel.h"

#include "ImGui/imgui.h"

OutlinePanel::OutlinePanel(const std::string& panelName)
	: EditorPanel(panelName)
{
}

void OutlinePanel::OnImGuiRender(Ref<Scene>& scene)
{
	ImGui::Begin(m_PanelName.c_str());
	m_Focused = ImGui::IsWindowFocused();
	m_Hovered = ImGui::IsWindowHovered();
	ImGui::End();
}
