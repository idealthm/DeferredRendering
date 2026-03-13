#pragma once
#include <string>

#include "RenderPass.h"
#include "FrameBuffer/FrameBuffer.h"

struct RenderContext;
class FrameBuffer;

class GBufferPass : public RenderPass
{
public:
	GBufferPass(uint32 width, uint32 height);
	virtual ~GBufferPass() = default;

	void Setup(RenderContext& ctx, FBAttachmentInfo & info) override;
	virtual void Execute(Ref<Scene> scene);
};
