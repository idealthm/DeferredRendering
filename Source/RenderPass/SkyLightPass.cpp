#include "SkyLightPass.h"

#include "Actor.h"
#include "Renderer.h"
#include "Scene.h"
#include "FrameBuffer/FrameBuffer.h"
#include "Shader/Shader.h"
#include "Model/Texture.h"

SkyLightPass::SkyLightPass()
{
	m_Shader = CreateRef<Shader>("Shaders/Passes/SkyBox", 1);
}

void SkyLightPass::Setup(FBAttachmentInfo& info, uint32_t step, RenderContext& ctx)
{
	info.DSS.depthTest = true;
	info.DSS.depthWrite = false;
	info.DSS.compareFunc = ECompareFunc::LEqual;

	info.Depth = {ctx.GBuffer_Depth->GetRendererID(), FBTextureLoadAction::Load, FBTextureStoreAction::Store};

	info.Attachments = {
		{ctx.LightMap_SceneColor->GetRendererID(), FBTextureLoadAction::Load, FBTextureStoreAction::Store}
	};
}

void SkyLightPass::Execute(Ref<Scene> scene, uint32_t step, RenderContext& ctx)
{
	m_Shader->Bind();

	uint32_t freeSlotIndex = m_Shader->GetFreeSlotIndex();
	ctx.ERP_Cubemap->Bind(freeSlotIndex);
	m_Shader->SetUniform1i("uCubeMap", freeSlotIndex++);

	m_UnitCube.Draw();
}

