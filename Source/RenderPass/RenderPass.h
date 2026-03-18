#pragma once
#include <vector>

#include "Common/Core.h"


struct FBAttachmentInfo;
struct FBTextureDesc;
class FrameBuffer;
struct RenderContext;
class Scene;
class Shader;

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

	virtual uint32 GetRenderTimes() {return 1;}

	virtual void Setup(FBAttachmentInfo& info, uint32 step);

	virtual void Execute(Ref<Scene> scene, uint32 step);
};
