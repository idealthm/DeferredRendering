#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Common/Core.h"


struct RenderContext;
class Scene;
class Shader;

class RenderPass
{
public:
	RenderPass(uint32 width, uint32 height)
		: m_Width(width), m_Height(height){}

	virtual ~RenderPass() = default;

	virtual uint32 GetDepthRendererID() const { return 0; }
	virtual uint32 GetColorAttachmentRendererID(const std::string& name) const { return 0; }

	virtual void OnWindowSizeChanged(int32 width, int32 height) {}

	virtual void PrePass(RenderContext& context) {}
	virtual void OnPass(RenderContext& context) {}
	virtual void PostPass(RenderContext& context) {}
protected:
	uint32 m_Width, m_Height;
};
