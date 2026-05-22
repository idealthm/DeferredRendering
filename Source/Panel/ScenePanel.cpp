#include "ScenePanel.h"

#include <ios>
#include <iostream>
#include <ostream>
#include <glm/gtc/type_ptr.inl>

#include "RenderPipeline.h"
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
	auto& finalColor = Renderer::Get().GetPipeline().GetContext().Final_SceneColor;
	// Bind
	// ImGui::Image((void*)(intptr_t)finalColor->GetHandle(), vec2, {0, 1}, {1, 0});
	if (uint32_t(vec2.x) != Renderer::Get().GetPipeline().GetContext().viewportSize.x || uint32_t(vec2.y) != Renderer::Get().GetPipeline().GetContext().viewportSize.y)
	{
		Renderer::Get().GetPipeline().GetContext().viewportSize = { vec2.x, vec2.y };
	}

	auto Actor = scene->GetActor<DirectionLightActor>();

	ImVec2 size = ImGui::GetWindowSize();
	ImVec2 pos = ImGui::GetWindowPos();

	// glm::mat4& view = Renderer::Get().GetPipeline().GetContext().FrameDataUB->Data.m_ViewProjection;
	// glm::mat4& projection = Renderer::Get().GetPipeline().GetContext().FrameDataUB->Data.m_Projection;
	glm::mat4 modelMatrix = Actor->GetTransform().GetModelMatrix(); // 你现在可以先 Mock 一个固定的 Actor

	ImGuizmo::SetRect(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
	ImGuizmo::SetDrawlist(); // 这一步确保它画在当前的窗口层

	// ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), (ImGuizmo::OPERATION)GetGizmoMode(), ImGuizmo::LOCAL, glm::value_ptr(modelMatrix));

	if (ImGuizmo::IsUsing()) {
		Actor->SetTransform(Math::Transform::FromMatrix(modelMatrix));
	}

	ImGui::End();
	ImGui::PopStyleVar();
}

uint32_t ScenePanel::GetGizmoMode() const
{
	static ImGuizmo::OPERATION s_GizmoMode [] = {ImGuizmo::TRANSLATE, ImGuizmo::ROTATE, ImGuizmo::SCALE};
	return s_GizmoMode[m_GizmoMode];
}

void ScenePanel::ToggleGizmoMode()
{
	m_GizmoMode = (m_GizmoMode + 1) % 3;
}
