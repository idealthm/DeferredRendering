#include "ExampleLayer.h"

#include "Actor.h"
#include "Renderer.h"
#include <Camera/Camera.h>

#include "Events/Event.h"
#include "Events/KeyEvent.h"
#include "Model/Texture.h"
#include "Shapes/MeshBuilder.h"
#include "glad/glad.h"
#include "ImGui/imgui.h"
#include "Lights/Light.h"
#include "Material/Material.h"
#include "Model/Util.h"
#include "RenderPass/ERPPass.h"


ExampleLayer::ExampleLayer(uint32 width, uint32 height)
	: Layer("ExampleLayer")
{
	m_Camera = CreateRef<Camera>();

	m_Scene = CreateRef<Scene>(width, height, m_Camera);
	auto PlaneActor = m_Scene->SpawnActor<StaticMeshActor>(glm::vec3(0.0f, -5.0f, 0.0f), glm::vec3(0, 0, 0), glm::vec3(50.f, 0.1f, 50.f));
	auto ModelA = m_Scene->SpawnActor<StaticMeshActor>();

	// auto Cube = m_Scene->SpawnActor<StaticMeshActor>(glm::vec3(1.0f, 5.0f, 0.0f), glm::vec3(-90.f, 0.0f, 0.0f), glm::vec3(2.0f, 2.0f, 2.0f));
	// Cube->SetStaticMesh(Util::MeshLoader::LoadAsset("Assets/objects/TreeStump/TreeStump.json"));

	Ref<StaticMesh> PlaneMesh = MeshBuilder::BuildCube(Material::CreateDefault());
	PlaneActor->SetStaticMesh(PlaneMesh);

	ModelA->SetStaticMesh(Util::MeshLoader::LoadAsset("Assets/objects/backpack/backpack.json"));

	m_Actor = m_Scene->SpawnActor<DirectionLightActor>(glm::vec3(0.f, 5.f, 0.f), glm::vec3(0, 0, 0), glm::vec3(1.f, 1.f, 1.f));
}

ExampleLayer::~ExampleLayer()
{
}

void ExampleLayer::OnDetach()
{
	Layer::OnDetach();
}

void ExampleLayer::OnUpdate(Timestep ts)
{
	m_Camera->OnUpdate(ts);

	RenderContext& ctx = m_Scene->GetRenderContext();
	auto& FrameData = ctx.FrameDataUB->Data;
	FrameData.m_Projection = m_Scene->GetProjectionMatrix();
	FrameData.m_ViewProjection = m_Camera->GetViewMatrix();
	FrameData.m_CameraPosition = m_Camera->GetPosition();
	FrameData.m_InvProjection = glm::inverse(m_Scene->GetProjectionMatrix());
	FrameData.m_InvViewProjection = glm::inverse(m_Scene->GetViewMatrix());

	ctx.FrameDataUB->Update();

	Renderer::Get().Render(m_Scene);
}

void ExampleLayer::OnImGuiRender()
{
	bool show_demo_window = true;
	ImGui::ShowDemoWindow(&show_demo_window);
}

void ExampleLayer::OnEvent(Event& event)
{
	m_Camera->OnEvent(event);

	EventDispatcher dispatcher(event);
	dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& e )
	{
		RenderContext& ctx = m_Scene->GetRenderContext();
		switch (e.GetKeyCode())
		{
			case Key::F1: ctx.FrameDataUB->Data.RenderMode = 1; return true;
			case Key::F2: ctx.FrameDataUB->Data.RenderMode = 2; return true;
			case Key::F3: ctx.FrameDataUB->Data.RenderMode = 3; return true;
			case Key::F4: ctx.FrameDataUB->Data.RenderMode = 4; return true;
			case Key::F5: ctx.FrameDataUB->Data.RenderMode = 5; return true;
			case Key::F6: ctx.FrameDataUB->Data.RenderMode = 6; return true;
			case Key::F7: ctx.FrameDataUB->Data.RenderMode = 7; return true;
		}
		return false;
	});
}

void ExampleLayer::OnWindowResize(uint32 width, uint32 height)
{
	m_Scene->SetWidth(width);
	m_Scene->SetHeight(height);
}

void ExampleLayer::OnAttach()
{

}
