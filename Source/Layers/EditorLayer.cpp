#include "EditorLayer.h"

#include "Actor.h"
#include "Application.h"
#include "Scene.h"
#include "Camera/CameraController.h"
#include "Events/Event.h"
#include "Events/MouseEvent.h"
#include "GLFW/glfw3.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "Model/Texture.h"
#include "Model/Util.h"
#include "Shapes/MeshBuilder.h"
#include "Windows/Window.h"
#include <Lights/Light.h>

#include "Camera/Camera.h"
#include "Events/Input.h"
#include "Events/KeyEvent.h"
#include "ImGui/ImGuizmo.h"
#include "Panel/OutlinePanel.h"
#include "Panel/PropertyPanel.h"
#include "Panel/ScenePanel.h"

class DirectionLightActor;

#define SCENE_PANEL_NAME "Game"
#define PROPERTY_PANEL_NAME "Properties"
#define OUTLINE_PANEL_NAME "Outline"


EditorLayer::EditorLayer()
	: Layer("EditorLayer")
{
	m_ScenePanel = CreateRef<ScenePanel>(SCENE_PANEL_NAME);
	m_PropertyPanel = CreateRef<PropertyPanel>(PROPERTY_PANEL_NAME);
	m_OutlinePanel = CreateRef<OutlinePanel>(OUTLINE_PANEL_NAME);

	// will load file future.
	LoadScene("...");

	m_EditorCamera = CreateRef<Camera>();
	m_CameraController = CreateRef<CameraController>();
}

void EditorLayer::LoadScene(std::string path)
{
	m_ScenePath = path;
	m_ActiveScene = CreateRef<Scene>();

	auto PlaneActor = m_ActiveScene->SpawnActor<StaticMeshActor>(glm::vec3(0.0f, -5.0f, 0.0f), glm::vec3(0, 0, 0), glm::vec3(50.f, 0.1f, 50.f));
	auto ModelA = m_ActiveScene->SpawnActor<StaticMeshActor>();

	// auto Cube = m_Scene->SpawnActor<StaticMeshActor>(glm::vec3(1.0f, 5.0f, 0.0f), glm::vec3(-90.f, 0.0f, 0.0f), glm::vec3(2.0f, 2.0f, 2.0f));
	// Cube->SetStaticMesh(Util::MeshLoader::LoadAsset("Assets/objects/TreeStump/TreeStump.json"));

	Ref<StaticMesh> PlaneMesh = MeshBuilder::BuildCube(Material::CreateDefault());
	PlaneActor->SetStaticMesh(PlaneMesh);

	ModelA->SetStaticMesh(Util::MeshLoader::LoadAsset("Assets/objects/backpack/backpack.json"));

	m_ActiveScene->SpawnActor<DirectionLightActor>(glm::vec3(0.f, 5.f, 0.f), glm::vec3(0, 0, 0), glm::vec3(1.f, 1.f, 1.f));
}

void EditorLayer::OnAttach()
{


	
}

void EditorLayer::OnDetach()
{
	Layer::OnDetach();
}

void EditorLayer::OnUpdate(Timestep ts)
{
	bool bSwitchToPlayer = Input::IsMouseButtonPressed(Mouse::ButtonRight) && m_EditorMode == EEditorMode::Editor;
	bool bSwitchToEditor = !Input::IsMouseButtonPressed(Mouse::ButtonRight) && m_EditorMode == EEditorMode::Player;

	if (bSwitchToPlayer || bSwitchToEditor)
		ToggleEditorMode();

	m_CameraController->OnUpdate(ts);

	auto& FrameData = g_ctx.FrameDataUB->Data;
	FrameData.m_Projection = m_EditorCamera->GetProjectionMatrix(1.0 * m_ViewportSize.x / m_ViewportSize.y);
	FrameData.m_ViewProjection = m_EditorCamera->GetViewMatrix();
	FrameData.m_CameraPosition = m_EditorCamera->GetPosition();
	FrameData.m_InvProjection = glm::inverse(FrameData.m_Projection);
	FrameData.m_InvViewProjection = glm::inverse(FrameData.m_ViewProjection);

	g_ctx.FrameDataUB->Update();

	Renderer::Get().Render(m_ActiveScene, m_ViewportSize);
}

void EditorLayer::OnEvent(Event& event)
{
	m_CameraController->OnEvent(event);

	EventDispatcher dispatcher(event);

	dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& e)
	{
		switch (e.GetKeyCode())
		{
			case Key::Space: m_ScenePanel->ToggleGizmoMode(); return true;
		}
		return false;
	});
}

void EditorLayer::ToggleEditorMode()
{
	auto window = Application::Get().GetWindow().GetNativeWindow();
	if (m_EditorMode == EEditorMode::Player)
	{
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		glfwSetCursorPos(window, x, y);
		m_EditorMode = EEditorMode::Editor;
		m_CameraController->ProcessCamera(nullptr);
		m_EditorCamera->bSkipEvent = true;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	else
	{
		m_EditorMode = EEditorMode::Player;
		m_CameraController->ProcessCamera(m_EditorCamera);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
}

void EditorLayer::OnImGuiRender()
{
	// Create settings
	m_MainDockingSpace = ImGui::GetID("My DockingSpace");
	
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	
	if (ImGui::DockBuilderGetNode(m_MainDockingSpace) == nullptr)
	{
		ImGui::DockBuilderAddNode(m_MainDockingSpace, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_PassthruCentralNode );
		ImGui::DockBuilderSetNodeSize(m_MainDockingSpace, viewport->Size);
		ImGuiID dock_id_left = 0;
		ImGuiID dock_id_main = m_MainDockingSpace;
		ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Left, 0.20f, &dock_id_left, &dock_id_main);
		ImGuiID dock_id_left_top = 0;
		ImGuiID dock_id_left_bottom = 0;
		ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Up, 0.50f, &dock_id_left_top, &dock_id_left_bottom);
		ImGui::DockBuilderDockWindow(SCENE_PANEL_NAME, dock_id_main);
		ImGui::DockBuilderDockWindow(PROPERTY_PANEL_NAME, dock_id_left_bottom);
		ImGui::DockBuilderDockWindow(OUTLINE_PANEL_NAME, dock_id_left_top);
		ImGui::DockBuilderFinish(m_MainDockingSpace);
	}

	// Submit dockspace
	ImGui::DockSpaceOverViewport(m_MainDockingSpace, viewport, ImGuiDockNodeFlags_PassthruCentralNode);

	m_ScenePanel->OnImGuiRender(m_ActiveScene);
	m_PropertyPanel->OnImGuiRender(m_ActiveScene);
	m_OutlinePanel->OnImGuiRender(m_ActiveScene);
}