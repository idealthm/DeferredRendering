#pragma once

#include <functional>
#include <mutex>
#include<string>

#include "Common/Core.h"
#include "Common/LayerStack.h"

class Window;
class WindowResizeEvent;
class WindowCloseEvent;
class Event;
class Layer;

struct ApplicationCommandLineArgs
{
	int Count = 0;
	char** Args = nullptr;

	const char* operator[](int index) const
	{
		ASSERT(index < Count);
		return Args[index];
	}
};

struct ApplicationSpecification
{
	std::string Name = "Hazel Application";
	std::string WorkingDirectory;
	ApplicationCommandLineArgs CommandLineArgs;
};

class Application
{
public:
	Application(const ApplicationSpecification& applicationSpecification);
	~Application();

	void Run();

	void OnEvent(Event& e);

	void PushLayer(Layer* layer);
	void PushOverlay(Layer* layer);

	Window& GetWindow() { return *m_Window; }

	void Close();

	static Application& Get() { return *s_Instance; }

	const ApplicationSpecification& GetSpecification() const { return m_Specification; }

	void SubmitToMainThread(const std::function<void()>& function);
private:
	bool OnWindowClose(WindowCloseEvent& e);
	bool OnWindowResize(WindowResizeEvent& e);

	void ExecuteMainThreadQueue();
private:
	ApplicationSpecification m_Specification;
	Scope<Window> m_Window;
	bool m_Running = true;
	bool m_Minimized = false;
	LayerStack m_LayerStack;
	float m_LastFrameTime = 0.0f;

	std::vector<std::function<void()>> m_MainThreadQueue;
	std::mutex m_MainThreadQueueMutex;
private:
	static Application* s_Instance;
};
