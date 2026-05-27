#pragma once
#include <memory>

#include "RenderPass.h"

class RenderTarget;
struct RenderContext;
class FrameBuffer;

class GBufferPass : public RenderPass
{
public:
	GBufferPass();

	void Setup(RenderContext& ctx) override;
	virtual void Execute(Ref<Scene> scene, RenderContext& ctx);

private:
	Ref<RenderTarget> m_RenderTarget;
};
