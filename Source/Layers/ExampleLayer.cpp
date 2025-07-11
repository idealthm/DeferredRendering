#include "ExampleLayer.h"

#include "Actor.h"
#include "Renderer.h"
#include <Camera/Camera.h>

#include "Events/Event.h"
#include "Events/KeyEvent.h"
#include "Model/Texture.h"
#include "RenderPass/DeferredPass.h"
#include "Shapes/MeshBuilder.h"
#include "glad/glad.h"
#include "Model/Util.h"


ExampleLayer::ExampleLayer(uint32 width, uint32 height)
	: Layer(width, height, "ExampleLayer")
{
	m_Camera = CreateRef<Camera>();

	m_Scene = CreateRef<Scene>(width, height, m_Camera);
	auto Cube = m_Scene->SpawnActor<StaticMeshActor>(glm::vec3(0.0f, 5.0f, 0.0f));
	auto Plane = m_Scene->SpawnActor<StaticMeshActor>(glm::vec3(0.0f, -5.0f, 0.0f), glm::vec3(0), glm::vec3(10.f, 0.1f, 10.f));
	auto ModelA = m_Scene->SpawnActor<StaticMeshActor>();

	uint32 white = 0xffffffff, red = 0xff0000ff;
	TextureDescription Desc;
	Desc.width = Desc.height = 1;
	Desc.format = GL_RGBA;
	Desc.bpp = 4;
	Ref<StaticMesh> CubeMesh = MeshBuilder::BuildCube(Texture2D::Create(Desc, &red));
	Cube->SetStaticMesh(CubeMesh);

	Ref<StaticMesh> PlaneMesh = MeshBuilder::BuildCube(Texture2D::Create(Desc, &white));
	Plane->SetStaticMesh(PlaneMesh);

	ModelA->SetStaticMesh(Util::MeshLoader::LoadAsset("Assets/objects/backpack/backpack.obj"));

	m_DeferredPass = Renderer::Get().AddPass<DeferredPass>(width, height);
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
	Renderer::Get().Draw(m_Scene);
}

void ExampleLayer::OnImGuiRender()
{
	Layer::OnImGuiRender();
	
}

void ExampleLayer::OnEvent(Event& event)
{
	m_Camera->OnEvent(event);

	EventDispatcher dispatcher(event);
	dispatcher.Dispatch<KeyPressedEvent>(BIND_FUNCTION_FN(ExampleLayer::OnKeyPress));
}

bool ExampleLayer::OnKeyPress(const KeyPressedEvent& event) const
{
	switch (event.GetKeyCode())
	{
		case Key::F1: m_DeferredPass->SetDebugMode(1); return true;
		case Key::F2: m_DeferredPass->SetDebugMode(2); return true;
		case Key::F3: m_DeferredPass->SetDebugMode(3); return true;
		case Key::F4: m_DeferredPass->SetDebugMode(4); return true;
		case Key::F5: m_DeferredPass->SetDebugMode(5); return true;
	}
	return false;
}

void ExampleLayer::OnAttach()
{

}
