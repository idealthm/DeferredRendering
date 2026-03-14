#pragma once
#include <string>

#include "Common/Core.h"
#include "Common/Timestep.h"

class Event;

class Layer
{
public:
	Layer(const std::string& name);
	virtual ~Layer() = default;

	virtual void OnAttach() {}
	virtual void OnDetach() {}
	virtual void OnUpdate(Timestep ts) {}
	virtual void OnImGuiRender() {}
	virtual void OnEvent(Event& event) {}

	virtual void OnWindowResize(uint32 width, uint32 height) {}

	const std::string& GetName() const { return m_DebugName; }
protected:
	std::string m_DebugName;
	uint32 m_Width, m_Height;
};
