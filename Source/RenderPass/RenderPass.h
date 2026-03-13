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
	RenderPass(uint32 width, uint32 height)
		: m_Width(width), m_Height(height){}

	virtual ~RenderPass() = default;

	virtual void Setup(RenderContext& ctx, FBAttachmentInfo & info);

	virtual void Execute(Ref<Scene> scene);
protected:
	uint32 m_Width, m_Height;
};
