#include "EditorLayer.h"

#include "Actor.h"
#include "Application.h"
#include "RenderPipeline.h"
#include "Scene.h"
#include "Camera/CameraController.h"
#include "Events/Event.h"

#include "Assimp/Util.h"
#include "Shapes/MeshBuilder.h"
#include "Windows/Window.h"
#include <Lights/Light.h>

#include "Engine.h"
#include "GLFW/glfw3.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "SkyLightActor.h"
#include "Camera/Camera.h"
#include "Events/Input.h"
#include "Events/KeyEvent.h"
#include "ImGui/ImGuizmo.h"
#include "Material/MaterialInstance.h"
#include "Panel/OutlinePanel.h"
#include "Panel/PropertyPanel.h"
#include "Panel/ScenePanel.h"
#include "RenderPass/EnvPreFilter.h"
#include "RenderPass/ERPPass.h"
#include "common/glmHelper.h"

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

	auto PlaneActor = m_ActiveScene->SpawnActor<StaticMeshActor>(glm::vec3(0.0f, -5.0f, 0.0f), glm::vec3(0, 0, 0), glm::vec3(10.f, 0.1f, 10.f));
	auto ModelA = m_ActiveScene->SpawnActor<StaticMeshActor>();

	// auto Cube = m_Scene->SpawnActor<StaticMeshActor>(glm::vec3(1.0f, 5.0f, 0.0f), glm::vec3(-90.f, 0.0f, 0.0f), glm::vec3(2.0f, 2.0f, 2.0f));
	// Cube->SetStaticMesh(Util::MeshLoader::LoadAsset("Assets/objects/TreeStump/TreeStump.json"));

	Ref<StaticMesh> PlaneMesh = MeshBuilder::BuildCube();
	PlaneActor->SetStaticMesh(PlaneMesh);
	auto mi = PlaneMesh->GetMaterial();

	mi->SetParameter("albedo", glm::vec3(0.3f, 0.4f, 0.5f));
	mi->SetParameter("roughness", 0.9f);
	mi->SetParameter("metallic", 0.1f);
	mi->SetParameter("ao", 0.1f);

	ModelA->SetStaticMesh(Util::MeshLoader::LoadAsset("Assets/objects/Gun/Gun.json"));
	// ModelA->SetStaticMesh(Util::MeshLoader::LoadAsset("Assets/objects/backpack/backpack.json"));

	m_ActiveScene->SpawnActor<DirectionLightActor>(glm::vec3(0.f, 5.f, 0.f), glm::vec3(-45.f, 0, 0), glm::vec3(1.f, 1.f, 1.f));
	m_ActiveScene->SpawnActor<SkyLightActor>();
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

	auto& ctx = RenderPipeline::Get().GetContext();
	auto& frameData = ctx.FrameDataUB.edit();

	float aspect = 1.0f * ctx.viewportSize.x / ctx.viewportSize.y;
	glm::mat4 proj = m_EditorCamera->GetProjectionMatrix(aspect);
	glm::mat4 view = m_EditorCamera->GetViewMatrix();

	frameData.clipFromViewMatrix = proj;
	frameData.viewFromClipMatrix = glm::inverse(proj);
	frameData.viewFromWorldMatrix = view;
	frameData.worldFromViewMatrix = glm::inverse(view);
	frameData.clipFromWorldMatrix = proj * view;
	frameData.worldFromClipMatrix = glm::inverse(proj * view);

	RenderPipeline::Get().Render(m_ActiveScene, RenderPipeline::Get().GetContext().viewportSize);
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

	ImGui::Begin("Settings");
	static bool VSync = true;
	if (ImGui::Checkbox("VSync", &VSync))
	{
		Application::Get().GetWindow().SetVSync(VSync);
	}
	float deltaTime = ImGui::GetIO().DeltaTime * 1000.0f;
	ImGui::Text("Frame Time: %.3f ms (%.1f FPS)", deltaTime, 1.0f / ImGui::GetIO().DeltaTime);
	ImGui::End();
}