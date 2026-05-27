#pragma once
#include <vector>

#include "Common/Core.h"
#include "RenderContext.h"


struct FBAttachmentInfo;
struct FBTextureDesc;
class FrameBuffer;
class Scene;
class Program;

enum class RenderPassType
{
	GBuffer,
	ShadowMap,
	Blur,
};

class RenderPass
{
public:
	RenderPass() = default;

	virtual ~RenderPass() = default;

	virtual uint32_t GetRenderTimes() {return 1;}

	virtual void Setup(RenderContext& ctx);

	virtual void Execute(Ref<Scene> scene, RenderContext& ctx);
};
