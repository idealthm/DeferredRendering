#include "LightingPass.h"

#include "Engine.h"
#include "Scene.h"
#include "Lights/Light.h"
#include "Material/MaterialInstance.h"
#include "Model/Texture.h"
#include "RHI/TextureSampler.h"
#include "Shader/Program.h"

using namespace TextureFactory;

LightingPass::LightingPass()
{
}

void LightingPass::Setup(RenderContext& ctx)
{
	
}

void LightingPass::Execute(Ref<Scene> scene, RenderContext& ctx)
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
	// ctx.LightDataUB.commit(gEngine->GetDriver());

	m_MaterialInstance->SetParameter("gPosition", ctx.GBuffer_Position, TextureSampler::LinearClamp());
	m_MaterialInstance->SetParameter("gNormal", ctx.GBuffer_Normal  , TextureSampler::LinearClamp());
	m_MaterialInstance->SetParameter("gAlbedo", ctx.GBuffer_Albedo, TextureSampler::LinearClamp());
	m_MaterialInstance->SetParameter("gMaterial", ctx.GBuffer_Material, TextureSampler::LinearClamp());

	m_MaterialInstance->SetParameter("uIBL_PreFilterMap", ctx.IBL_PreFilterMap, TextureSampler::LinearClamp());
	m_MaterialInstance->SetParameter("gShadowMap", ctx.ShadowMap_Depth, TextureSampler::Shadow());
	m_MaterialInstance->SetParameter("uIrradianceMap", ctx.IBL_IrradianceMap, TextureSampler::LinearClamp());
	m_MaterialInstance->SetParameter("uBRDF_LUT", ctx.BRDF_LUT, TextureSampler::LinearClamp());

	// TODO: set up pipeline state and call driver.draw() with m_ScreenQuad
}