#include "ExampleLayer.h"

#include "Actor.h"
#include "Renderer.h"
#include <Camera/Camera.h>

#include "Events/Event.h"
#include "Events/KeyEvent.h"
#include "Model/Texture.h"
#include "Shapes/MeshBuilder.h"
#include "glad/glad.h"
#include "Lights/Light.h"
#include "Model/Util.h"


ExampleLayer::ExampleLayer(uint32 width, uint32 height)
	: Layer(width, height, "ExampleLayer")
{
	m_Camera = CreateRef<Camera>();

	m_Scene = CreateRef<Scene>(width, height, m_Camera);
	auto Cube = m_Scene->SpawnActor<StaticMeshActor>(glm::vec3(1.0f, 5.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(2.0f, 2.0f, 2.0f));
	auto PlaneActor = m_Scene->SpawnActor<StaticMeshActor>(glm::vec3(0.0f, -5.0f, 0.0f), glm::vec3(0, 0, 0), glm::vec3(100.f, 0.1f, 100.f));
	auto ModelA = m_Scene->SpawnActor<StaticMeshActor>();

	uint32 white = 0xffffffff, red = 0xff0000ff;
	TextureDescription Desc;
	Desc.width = Desc.height = 1;
	Desc.format = GL_RGBA;
	Desc.bpp = 4;
	Ref<StaticMesh> CubeMesh = MeshBuilder::BuildCube(Texture2D::Create(Desc, &red));
	Cube->SetStaticMesh(CubeMesh);

	Ref<StaticMesh> PlaneMesh = MeshBuilder::BuildCube(Texture2D::Create(Desc, &white));
	PlaneActor->SetStaticMesh(PlaneMesh);

	ModelA->SetStaticMesh(Util::MeshLoader::LoadAsset("Assets/objects/backpack/backpack.obj"));

	m_Actor = m_Scene->SpawnActor<DirectionLightActor>(glm::vec3(0.f, 0.f, 0.f), glm::vec3(-30.f, 0.f, 0.f));
	// m_Actor->AddComponent<StaticMeshComponent>()->SetMesh(CubeMesh);
	// auto PointLight = m_Scene->SpawnActor<PointLightActor>(glm::vec3(5.f));
	// auto SpotLight = m_Scene->SpawnActor<SpotLightActor>(glm::vec3(-5.f, 5.f, -5.f), glm::vec3(0.f, -30.f, 0.f));
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

	static float CachedTime = 0.f;
	CachedTime += ts;
	m_Actor->SetRotation(glm::vec3(- 10 * CachedTime, 0, 0));
	CachedTime = CachedTime > 18 ? CachedTime - 18 : CachedTime;

	std::cout << CachedTime * 10 << std::endl;

	Renderer::Get().Draw(m_Scene);
}

void ExampleLayer::OnImGuiRender()
{
	Layer::OnImGuiRender();
}

void ExampleLayer::OnEvent(Event& event)
{
	m_Camera->OnEvent(event);
}

void ExampleLayer::OnAttach()
{

}
