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

	virtual void Setup(FBAttachmentInfo & info);

	virtual void Execute(Ref<Scene> scene);
};
