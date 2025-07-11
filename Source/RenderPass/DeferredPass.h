#pragma once
#include <string>

#include "RenderPass.h"

class FrameBuffer;

class DeferredPass : public RenderPass
{
public:
	DeferredPass(int32 width, int32 height);
	~DeferredPass();

	void SetDebugMode(uint32 mode);

	virtual void OnWindowSizeChanged(int32 width, int32 height);

	virtual void PrePass(const Ref<Scene>& scene) override;
	virtual void OnPass(const Ref<Scene>& scene) override;
	virtual void PostPass(const Ref<Scene>& scene) override;

	void BindForReading(const std::string& name, int32 index, int32 Slot) const;

private:
	uint32 DebugMode;

	Ref<Shader> GeometryShader;
	Ref<Shader> LightShader;

	Ref<FrameBuffer> m_FrameBuffer;
};
