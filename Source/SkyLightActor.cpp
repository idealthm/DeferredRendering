#include "SkyLightActor.h"

#include "RenderPipeline.h"
#include "Component/SkyComponent.h"
#include "RenderPass/CubeMapConvolution.h"
#include "RenderPass/EnvPreFilter.h"
#include "RenderPass/ERPPass.h"

SkyLightActor::SkyLightActor()
{
	m_SkyComponent = AddComponent<SkyComponent>();
	SetRootComponent(m_SkyComponent);
}

void SkyLightActor::OnSpawn()
{
	RenderPipeline::Get().StartPass(m_WeakScene.lock(), CreateRef<ERPPass>("Assets/textures/hdr/newport_loft.hdr", 512), {512, 512});
	RenderPipeline::Get().StartPass(m_WeakScene.lock(), CreateRef<CubeMapConvolution>(), {32, 32});
	RenderPipeline::Get().StartPass(m_WeakScene.lock(), CreateRef<EnvPreFilter>(), {32, 32});
}

