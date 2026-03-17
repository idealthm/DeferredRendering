#include "ScenePanel.h"

#include <ios>
#include <iostream>
#include <ostream>
#include <glm/gtc/type_ptr.inl>

#include "Renderer.h"
#include "ImGui/imgui.h"
#include "ImGui/ImGuizmo.h"
#include "Lights/Light.h"

ScenePanel::ScenePanel(const std::string& panelName)
	: EditorPanel(panelName)
{
}

void ScenePanel::OnImGuiRender(Ref<Scene>& scene)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
	ImGui::Begin(m_PanelName.c_str());

	m_Focused = ImGui::IsWindowFocused();
	m_Hovered = ImGui::IsWindowHovered();
	ImVec2 vec2 = ImGui::GetContentRegionAvail();
	uint32 texId = g_ctx.Final_SceneColor->GetRendererID();
	ImGui::Image(texId, vec2, {0, 1}, {1, 0});
	if (uint32(vec2.x) != g_ctx.viewportSize.x || uint32(vec2.y) != g_ctx.viewportSize.y)
	{
		g_ctx.viewportSize = { vec2.x, vec2.y };
	}

	auto Actor = scene->GetActor<DirectionLightActor>();

	ImVec2 size = ImGui::GetWindowSize();
	ImVec2 pos = ImGui::GetWindowPos();

	glm::mat4& view = g_ctx.FrameDataUB->Data.m_ViewProjection;
	glm::mat4& projection = g_ctx.FrameDataUB->Data.m_Projection;
	glm::mat4 modelMatrix = Actor->GetTransform().GetModelMatrix(); // 你现在可以先 Mock 一个固定的 Actor

	ImGuizmo::SetRect(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
	ImGuizmo::SetDrawlist(); // 这一步确保它画在当前的窗口层

	ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), (ImGuizmo::OPERATION)GetGizmoMode(), ImGuizmo::LOCAL, glm::value_ptr(modelMatrix));

	if (ImGuizmo::IsUsing()) {
		Actor->SetTransform(Math::Transform::FromMatrix(modelMatrix));
	}

	ImGui::End();
	ImGui::PopStyleVar();
}

uint32 ScenePanel::GetGizmoMode() const
{
	static ImGuizmo::OPERATION s_GizmoMode [] = {ImGuizmo::TRANSLATE, ImGuizmo::ROTATE, ImGuizmo::SCALE};
	return s_GizmoMode[m_GizmoMode];
}

void ScenePanel::ToggleGizmoMode()
{
	m_GizmoMode = (m_GizmoMode + 1) % 3;
}
