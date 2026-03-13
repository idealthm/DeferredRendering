#include "ExampleLayer.h"

#include "Actor.h"
#include "Renderer.h"
#include <Camera/Camera.h>

#include "Model/Texture.h"
#include "Shapes/MeshBuilder.h"
#include "glad/glad.h"
#include "Lights/Light.h"
#include "Material/Material.h"
#include "Model/Util.h"
#include "RenderPass/ERPPass.h"


ExampleLayer::ExampleLayer(uint32 width, uint32 height)
	: Layer(width, height, "ExampleLayer")
{
	m_Camera = CreateRef<Camera>();

	m_Scene = CreateRef<Scene>(width, height, m_Camera);
	auto Cube = m_Scene->SpawnActor<StaticMeshActor>(glm::vec3(1.0f, 5.0f, 0.0f), glm::vec3(-90.f, 0.0f, 0.0f), glm::vec3(2.0f, 2.0f, 2.0f));
	auto PlaneActor = m_Scene->SpawnActor<StaticMeshActor>(glm::vec3(0.0f, -5.0f, 0.0f), glm::vec3(0, 0, 0), glm::vec3(100.f, 0.1f, 100.f));
	auto ModelA = m_Scene->SpawnActor<StaticMeshActor>();

	Cube->SetStaticMesh(Util::MeshLoader::LoadAsset("Assets/objects/TreeStump/TreeStump.json"));

	Ref<StaticMesh> PlaneMesh = MeshBuilder::BuildCube(Material::CreateDefault());
	PlaneActor->SetStaticMesh(PlaneMesh);

	ModelA->SetStaticMesh(Util::MeshLoader::LoadAsset("Assets/objects/backpack/backpack.json"));

	m_Actor = m_Scene->SpawnActor<DirectionLightActor>(glm::vec3(0.f, 0.f, 0.f), glm::vec3(-30.f, 0.f, 0.f));

	
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
	Layer::OnImGuiRender();
}

void ExampleLayer::OnEvent(Event& event)
{
	m_Camera->OnEvent(event);
}

void ExampleLayer::OnWindowResize(uint32 width, uint32 height)
{
	m_Scene->SetWidth(width);
	m_Scene->SetHeight(height);
}

void ExampleLayer::OnAttach()
{

}
