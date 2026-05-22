#include "LightingPass.h"

#include "Engine.h"
#include "Scene.h"
#include "FrameBuffer/FrameBuffer.h"
#include "Lights/Light.h"
#include "Material/MaterialInstance.h"
#include "Model/Texture.h"
#include "RHI/TextureSampler.h"
#include "Shader/Program.h"

using namespace TextureFactory;

LightingPass::LightingPass()
{
}

void LightingPass::Setup(FBAttachmentInfo& info, uint32_t step, RenderContext& ctx)
{
	// Lighting Pass 是全屏绘制（Full-screen Quad），通常不需要深度测试
	info.Depth = {};

	info.DSS.depthWrite = false;
	info.DSS.depthTest = false;
	info.DSS.compareFunc = ECompareFunc::Less;

	CreateResource(ctx.LightMap_SceneColor, CreateHDRBuffer(info.Width, info.Height));

	// 输入：此时 ctx.GBuffer_Normal 等纹理已由前面 Pass 生成
	// 输出：如果前面 Skybox 已经画了，这里 LoadAction 应该是 Load，否则会覆盖天空
	info.Attachments = {
		// { ctx.LightMap_SceneColor->GetRendererID(), FBTextureLoadAction::Load, FBTextureStoreAction::Store }
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
				LightInfo& info = ctx.LightDataUB.edit().lights[index++];
				info.position = Light->GetLocation();
				info.color = Light->GetColor();
				info.type = 0;
				info.intensity = Light->GetIntensity() * 10;
				info.direction = Light->GetDirection();
			}
		}
	}
	ctx.LightDataUB.edit().NumLights = index;
	ctx.LightDataUB.commit(gEngine->GetDriver());

	m_MaterialInstance->SetParameter("gPosition", ctx.GBuffer_Position->GetHandle(), TextureSampler::LinearClamp());
	m_MaterialInstance->SetParameter("gNormal", ctx.GBuffer_Normal->GetHandle()  , TextureSampler::LinearClamp());
	m_MaterialInstance->SetParameter("gAlbedo", ctx.GBuffer_Albedo->GetHandle(), TextureSampler::LinearClamp());
	m_MaterialInstance->SetParameter("gMaterial", ctx.GBuffer_Material->GetHandle(), TextureSampler::LinearClamp());

	m_MaterialInstance->SetParameter("uIBL_PreFilterMap", ctx.IBL_PreFilterMap->GetHandle(), TextureSampler::LinearClamp());
	m_MaterialInstance->SetParameter("gShadowMap", ctx.ShadowMap_Depth->GetHandle(), TextureSampler::Shadow());
	m_MaterialInstance->SetParameter("uIrradianceMap", ctx.IBL_IrradianceMap->GetHandle(), TextureSampler::LinearClamp());
	m_MaterialInstance->SetParameter("uBRDF_LUT", ctx.BRDF_LUT->GetHandle(), TextureSampler::LinearClamp());

	// TODO: set up pipeline state and call driver.draw() with m_ScreenQuad
}