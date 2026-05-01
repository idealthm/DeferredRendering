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
	void OnWindowResize(uint32_t width, uint32_t height) override;

	void OnAttach() override;
	void OnDetach() override;

	void Begin();
	void End();

	void SetDarkThemeColors();

	void SetBlockEvents(bool block) {m_BlockEvents = block;}

	uint32_t GetActiveWidgetID() const;
private:
	bool m_BlockEvents = false;
};
