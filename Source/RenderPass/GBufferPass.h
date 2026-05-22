#pragma once
#include <memory>

#include "RenderPass.h"
#include "FrameBuffer/FrameBuffer.h"

struct RenderContext;
class FrameBuffer;

class GBufferPass : public RenderPass
{
public:
	GBufferPass();

	void Setup(FBAttachmentInfo& info, uint32_t step, RenderContext& ctx) override;
	virtual void Execute(Ref<Scene> scene, uint32_t step, RenderContext& ctx);
};
