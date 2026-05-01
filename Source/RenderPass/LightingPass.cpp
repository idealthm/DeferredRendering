#include "LightingPass.h"

#include "Scene.h"
#include "FrameBuffer/FrameBuffer.h"
#include "Lights/Light.h"
#include "Model/Texture.h"
#include "RHI/SamplerPool.h"
#include "Shader/Shader.h"
#include "glad/glad.h"
#include "Shader/ShaderLibrary.h"

LightingPass::LightingPass()
{
	m_Shader = ShaderLibrary::Get().GetShader("Shaders/DirectionLight", nullptr);
}

void LightingPass::Setup(FBAttachmentInfo& info, uint32_t step, RenderContext& ctx)
{
	// Lighting Pass 是全屏绘制（Full-screen Quad），通常不需要深度测试
	info.Depth = {};

	info.DSS.depthWrite = false;
	info.DSS.depthTest = false;
	info.DSS.compareFunc = ECompareFunc::Less;

	CreateResource(ctx.LightMap_SceneColor, CreateHDRBuffer(info.Width, info.Height));

	if (!ctx.LightMap_SceneColor->GetSampler())
		ctx.LightMap_SceneColor->SetSampler(SamplerPool::Get().GetOrCreate(DefaultClampSampler()));

	// 输入：此时 ctx.GBuffer_Normal 等纹理已由前面 Pass 生成
	// 输出：如果前面 Skybox 已经画了，这里 LoadAction 应该是 Load，否则会覆盖天空
	info.Attachments = {
		{ ctx.LightMap_SceneColor->GetRendererID(), FBTextureLoadAction::Load, FBTextureStoreAction::Store }
	};
}

void LightingPass::Execute(Ref<Scene> scene, uint32_t step, RenderContext& ctx)
{
	int32_t index = 0;
	for (auto lightActor : scene->GetActors())
	{
		for (auto& Comp : lightActor->GetComponents())
		{
			if (auto Light = std::dynamic_pointer_cast<DirectionLightComponent>(Comp))
			{
				LightInfo& info = ctx.LightDataUB->Data.lights[index++];
				info.position = Light->GetLocation();
				info.color = Light->GetColor();
				info.type = 0;
				info.intensity = Light->GetIntensity() * 10;
				info.direction = Light->GetDirection();
			}
		}
	}
	ctx.LightDataUB->Data.NumLights = index;
	ctx.LightDataUB->Update();

	uint32_t freeSlot = m_Shader->GetFreeSlotIndex();

	m_Shader->Bind();
	ctx.GBuffer_Position->Bind(freeSlot);
	m_Shader->SetUniform1i("gPosition", freeSlot++);

	ctx.GBuffer_Normal->Bind(freeSlot);
	m_Shader->SetUniform1i("gNormal", freeSlot++);

	ctx.GBuffer_Albedo->Bind(freeSlot);
	m_Shader->SetUniform1i("gAlbedo", freeSlot++);

	ctx.GBuffer_Material->Bind(freeSlot);
	m_Shader->SetUniform1i("gMaterial", freeSlot++);

	ctx.ShadowMap_Depth->Bind(freeSlot);
	m_Shader->SetUniform1i("gShadowMap", freeSlot++);

	ctx.IBL_IrradianceMap->Bind(freeSlot);
	m_Shader->SetUniform1i("uIrradianceMap", freeSlot++);

	ctx.BRDF_LUT->Bind(freeSlot);
	m_Shader->SetUniform1i("uBRDF_LUT", freeSlot++);

	ctx.IBL_PreFilterMap->Bind(freeSlot);
	m_Shader->SetUniform1i("uIBL_PreFilterMap", freeSlot++);

	m_ScreenQuad.Draw();
}