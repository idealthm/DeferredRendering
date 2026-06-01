#include "ScenePanel.h"

#include <ios>
#include <iostream>
#include <ostream>
#include <glm/gtc/type_ptr.inl>

#include "RenderPipeline.h"
#include "ImGui/imgui.h"
#include "ImGui/ImGuizmo.h"
#include "ImGuiHelper.h"
#include "glad/glad.h"
#include "Lights/Light.h"
#include "Model/Texture.h"

namespace {

void ImDisableBlend(const ImDrawList*, const ImDrawCmd*) { glDisable(GL_BLEND); }
void ImEnableBlend(const ImDrawList*, const ImDrawCmd*)  { glEnable(GL_BLEND); }

} // namespace

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

	auto& finalColor = RenderPipeline::Get().GetContext().GBuffer_Albedo;

	auto* dl = ImGui::GetWindowDrawList();
	ImVec2 cursor = ImGui::GetCursorScreenPos();
	dl->AddCallback(ImDisableBlend, nullptr);
	dl->AddImage(ToImTexture(finalColor), cursor, ImVec2{cursor.x + vec2.x, cursor.y + vec2.y}, {0, 1}, {1, 0});
	dl->AddCallback(ImEnableBlend, nullptr);
	ImGui::Dummy(vec2);

	if (uint32_t(vec2.x) != RenderPipeline::Get().GetContext().viewportSize.x || uint32_t(vec2.y) != RenderPipeline::Get().GetContext().viewportSize.y)
	{
		RenderPipeline::Get().GetContext().viewportSize = { vec2.x, vec2.y };
	}

	auto Actor = scene->GetActor<DirectionLightActor>();

	ImVec2 size = ImGui::GetWindowSize();
	ImVec2 pos = ImGui::GetWindowPos();

	glm::mat4 modelMatrix = Actor->GetTransform().GetModelMatrix();

	ImGuizmo::SetRect(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
	ImGuizmo::SetDrawlist();

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
