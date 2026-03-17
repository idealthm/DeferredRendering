#pragma once
#include "Layer.h"

class ImGuiLayer : public Layer
{
public:
	ImGuiLayer();

	~ImGuiLayer() override;

	void OnUpdate(Timestep ts) override;
	void OnImGuiRender() override;
	void OnEvent(Event& event) override;
	void OnWindowResize(uint32 width, uint32 height) override;

	void OnAttach() override;
	void OnDetach() override;

	void Begin();
	void End();

	void SetDarkThemeColors();

	void SetBlockEvents(bool block) {m_BlockEvents = block;}

	uint32 GetActiveWidgetID() const;
private:
	bool m_BlockEvents = false;
};
