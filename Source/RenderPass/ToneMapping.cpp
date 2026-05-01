#include "ToneMapping.h"

#include "Renderer.h"
#include "Scene.h"
#include "FrameBuffer/FrameBuffer.h"
#include "glad/glad.h"
#include "Model/Texture.h"
#include "RHI/SamplerPool.h"
#include "Shader/Shader.h"
#include "Shader/ShaderLibrary.h"

ToneMapping::ToneMapping()
{
	m_Shader = ShaderLibrary::Get().GetShader("Shaders/Passes/ToneMapping", nullptr);
}

void ToneMapping::Setup(FBAttachmentInfo& info, uint32_t step, RenderContext& ctx)
{
	info.Depth = {};
 
	info.DSS.depthTest = false;
	info.DSS.depthWrite = false;

	CreateResource(ctx.Final_SceneColor, CreateFinalColor(info.Width, info.Height));

	if (!ctx.Final_SceneColor->GetSampler())
		ctx.Final_SceneColor->SetSampler(SamplerPool::Get().GetOrCreate(DefaultClampSampler()));

	info.Attachments = {
		{ctx.Final_SceneColor->GetRendererID(), FBTextureLoadAction::Clear, FBTextureStoreAction::Store},
	};
}

void ToneMapping::Execute(Ref<Scene> scene, uint32_t step, RenderContext& ctx)
{
	m_Shader->Bind();

	int32_t freeIndex = m_Shader->GetFreeSlotIndex();

	ctx.LightMap_SceneColor->Bind(freeIndex);
	m_Shader->SetUniform1i("uHdrSceneColor", freeIndex++);

	m_ScreenQuad.Draw();
}
