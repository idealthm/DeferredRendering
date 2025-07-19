#pragma once
#include "Layer.h"
#include "Scene.h"
#include "Common/Core.h"

class StaticMeshActor;
class KeyTypedEvent;
class DeferredPass;
class KeyPressedEvent;

class ExampleLayer : public Layer
{
public:
	ExampleLayer(uint32 width, uint32 height);
	~ExampleLayer() override;

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(Timestep ts) override;
	void OnImGuiRender() override;
	void OnEvent(Event& event) override;

private:
	Ref<Scene>				m_Scene;
	std::shared_ptr<Camera> m_Camera;

	Ref<Actor>				m_Actor;
};
