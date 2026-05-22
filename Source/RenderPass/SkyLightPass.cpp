#include "SkyLightPass.h"

#include "Actor.h"
#include "Renderer.h"
#include "Scene.h"
#include "FrameBuffer/FrameBuffer.h"
#include "Model/Texture.h"

SkyLightPass::SkyLightPass()
{
}

void SkyLightPass::Setup(FBAttachmentInfo& info, uint32_t step, RenderContext& ctx)
{
	info.DSS.depthTest = true;
	info.DSS.depthWrite = false;
	info.DSS.compareFunc = ECompareFunc::LEqual;

	(void)info.Depth; //  = {ctx.GBuffer_Depth->GetRendererID(), FBTextureLoadAction::Load, FBTextureStoreAction::Store};

	info.Attachments = {
		// {ctx.LightMap_SceneColor->GetRendererID(), FBTextureLoadAction::Load, FBTextureStoreAction::Store}
	};
}

void SkyLightPass::Execute(Ref<Scene> scene, uint32_t step, RenderContext& ctx)
{
	// TODO: set up pipeline state and call driver.draw() with m_UnitCube
}

