#include "SkyLightActor.h"

#include "Renderer.h"
#include "Component/SkyComponent.h"
#include "RenderPass/CubeMapConvolution.h"
#include "RenderPass/ERPPass.h"

SkyLightActor::SkyLightActor()
{
	m_SkyComponent = AddComponent<SkyComponent>();
	SetRootComponent(m_SkyComponent);
	m_ERPPass = CreateRef<ERPPass>("Assets/textures/hdr/newport_loft.hdr", 512);
	m_CubeInvolution = CreateRef<CubeMapConvolution>();
}

void SkyLightActor::OnSpawn()
{
	Renderer::Get().StartPass(m_WeakScene.lock(), m_ERPPass, {512, 512});
	Renderer::Get().StartPass(m_WeakScene.lock(), m_CubeInvolution, {32, 32});
}

