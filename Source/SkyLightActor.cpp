#include "SkyLightActor.h"

#include "Renderer.h"
#include "Component/SkyComponent.h"
#include "RenderPass/ERPPass.h"

SkyLightActor::SkyLightActor()
{
	m_SkyComponent = AddComponent<SkyComponent>();
	SetRootComponent(m_SkyComponent);
	m_ERPPass = CreateRef<ERPPass>("Assets/textures/hdr/newport_loft.hdr", 512);
}

void SkyLightActor::OnSpawn()
{
	Renderer::Get().StartPass(m_WeakScene.lock(), m_ERPPass, {512, 512});
}

