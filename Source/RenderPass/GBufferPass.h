#pragma once
#include <string>

#include "RenderPass.h"
#include "FrameBuffer/FrameBuffer.h"

struct RenderContext;
class FrameBuffer;

class GBufferPass : public RenderPass
{
public:
	GBufferPass();
	virtual ~GBufferPass() = default;

	void Setup(FBAttachmentInfo & info) override;
	virtual void Execute(Ref<Scene> scene);
};
