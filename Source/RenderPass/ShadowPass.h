#pragma once
#include "RenderPass.h"

class FrameBuffer;

class ShadowPass: public RenderPass
{
public:
	ShadowPass(uint32 width, uint32 height);
	~ShadowPass();

	void Setup(RenderContext& ctx, FBAttachmentInfo & info) override;
	void Execute(Ref<Scene> scene) override;
};
