#pragma once

#include <functional>
#include <memory>
#include <string>

#include "Common/Core.h"
#include "Events/Event.h"
#include "GraphicContext.h"

class GraphicContext;
class Scene;

struct WindowProperties
{
	std::string title;
	uint32		width;
	uint32		height;
};

class Window
{
public:
	using EventCallbackFn = std::function<void(Event&)>;

	Window(const WindowProperties& Props);
	~Window();

	void Update() const;

	int32 GetWidth() const {return m_Data.Width;}
	int32 GetHeight() const {return m_Data.Height;}

	void SetEventCallback(const EventCallbackFn& callback);
	void SetVSync(bool enabled);
	bool IsVSync() const;

	GLFWwindow* GetNativeWindow() const {return m_Window;}
private:
	void Init(const WindowProperties& Props);
	
	void Shutdown() const;
private:
	struct WindowData
	{
		std::string Title;
		unsigned int Width, Height;
		bool VSync;

		EventCallbackFn EventCallback;
	};

	WindowData	m_Data;
	GLFWwindow* m_Window;
	Scope<GraphicContext> m_Context;
};
