#pragma once
#include <memory>
#include <vector>

#include "RenderPass.h"

class RenderTarget;
struct RenderContext;
struct PerRenderableData;

namespace RHI { struct HwBufferObject; }

class GBufferPass : public RenderPass
{
public:
	GBufferPass();

	void Setup(RenderContext& ctx) override;
	virtual void Execute(Ref<Scene> scene, RenderContext& ctx);

private:
	Ref<RenderTarget> m_RenderTarget;
	DescriptorSet m_DescriptorSetPerRender;
	Handle<RHI::HwBufferObject> m_ModelDataHandle;
	std::vector<PerRenderableData> m_PerRenderableData;
};
