#include "SkyLightPass.h"

#include "Actor.h"
#include "Renderer.h"
#include "Scene.h"
#include "FrameBuffer/FrameBuffer.h"
#include "Shader/Shader.h"
#include "Model/Texture.h"
#include "Shapes/MeshBuilder.h"

SkyLightPass::SkyLightPass()
{
	m_Shader = CreateRef<Shader>("Shaders/Passes/SkyBox", 1);
}

void SkyLightPass::Setup(FBAttachmentInfo& info, uint32 step)
{
	info.DSS.depthTest = true;
	info.DSS.depthWrite = false;
	info.DSS.compareFunc = ECompareFunc::LEqual;

	info.Depth = {g_ctx.GBuffer_Depth->GetRendererID(), FBTextureLoadAction::Load, FBTextureStoreAction::Store};

	info.Attachments = {
		{g_ctx.LightMap_SceneColor->GetRendererID(), FBTextureLoadAction::Load, FBTextureStoreAction::Store}
	};
}

void SkyLightPass::Execute(Ref<Scene> scene, uint32 step)
{
	m_Shader->Bind();

	uint32 freeSlotIndex = m_Shader->GetFreeSlotIndex();
	g_ctx.ERP_Cubemap->Bind(freeSlotIndex);
	m_Shader->SetUniform1i("uCubeMap", freeSlotIndex++);

	MeshBuilder::BuildCube(Material::CreateDefault())->GetMeshSections()[0]->Draw();
}

