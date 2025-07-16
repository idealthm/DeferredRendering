#include "Application.h"

#include <filesystem>
#include <fstream>

#include "Layers/Layer.h"
#include "Renderer.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Layers/ExampleLayer.h"
#include "Model/StaticMesh.h"
#include "Windows/Window.h"
#include "GLFW/glfw3.h"

using namespace std;


constexpr unsigned int SCR_WIDTH = 1600;
constexpr unsigned int SCR_HEIGHT = 900;

Application* Application::s_Instance = nullptr;

Application::Application(const ApplicationSpecification& specification)
	: m_Specification(specification)
{
	ASSERT(!s_Instance);
	s_Instance = this;

	// Set working directory here
	if (!m_Specification.WorkingDirectory.empty())
		std::filesystem::current_path(m_Specification.WorkingDirectory);

	m_Window = CreateScope<Window>(WindowProperties{m_Specification.Name, SCR_WIDTH, SCR_HEIGHT});
	m_Window->SetEventCallback([this](auto&&... args) -> decltype(auto)
	{
		return this->OnEvent(std::forward<decltype(args)>(args)...);
	});

	Renderer::Get().Init(SCR_WIDTH, SCR_HEIGHT);

	PushLayer(new ExampleLayer(SCR_WIDTH, SCR_HEIGHT));
}

Application::~Application()
{
	Renderer::Get().Shutdown();  
}

void Application::PushLayer(Layer* layer)
{
	m_LayerStack.PushLayer(layer);
	layer->OnAttach();
}

void Application::PushOverlay(Layer* layer)
{
	m_LayerStack.PushOverlay(layer);
	layer->OnAttach();
}

void Application::Close()
{
	m_Running = false;
}

void Application::SubmitToMainThread(const std::function<void()>& function)
{
	std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

	m_MainThreadQueue.emplace_back(function);
}

void Application::OnEvent(Event& e)
{
	EventDispatcher dispatcher(e);
	dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& e )
	{
		switch (e.GetKeyCode())
		{
			case Key::Escape: m_Running = false; return true;
			case Key::F1: Renderer::Get().m_RenderMode = 1; return true;
			case Key::F2: Renderer::Get().m_RenderMode = 2; return true;
			case Key::F3: Renderer::Get().m_RenderMode = 3; return true;
			case Key::F4: Renderer::Get().m_RenderMode = 4; return true;
			case Key::F5: Renderer::Get().m_RenderMode = 5; return true;
			case Key::F6: Renderer::Get().m_RenderMode = 6; return true;
			case Key::F7: Renderer::Get().m_RenderMode = 7; return true;
		}
		return false;
	});
	dispatcher.Dispatch<WindowCloseEvent>(BIND_FUNCTION_FN(Application::OnWindowClose));
	dispatcher.Dispatch<WindowResizeEvent>(BIND_FUNCTION_FN(Application::OnWindowResize));

	for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
	{
		if (e.Handled) 
			break;
		(*it)->OnEvent(e);
	}
}

void Application::Run()
{
	while (m_Running)
	{
		float time = Time::GetTime();
		Timestep timestep = time - m_LastFrameTime;
		m_LastFrameTime = time;

		ExecuteMainThreadQueue();

		if (!m_Minimized)
		{
			{
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(timestep);
			}
		}

		m_Window->Update();
	}
}

bool Application::OnWindowClose(WindowCloseEvent& e)
{
	m_Running = false;
	return true;
}

bool Application::OnWindowResize(WindowResizeEvent& e)
{
	if (e.GetWidth() == 0 || e.GetHeight() == 0)
	{
		m_Minimized = true;
		return false;
	}

	m_Minimized = false;
	Renderer::Get().OnWindowResize(e.GetWidth(), e.GetHeight());

	for (auto& layer : m_LayerStack)
	{
		layer->OnWindowResize(e.GetWidth(), e.GetHeight());
	}

	return false;
}

void Application::ExecuteMainThreadQueue()
{
	std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

	for (auto& func : m_MainThreadQueue)
		func();

	m_MainThreadQueue.clear();
}