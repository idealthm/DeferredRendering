#include "SkyLightActor.h"

#include <utility>

#include "Renderer.h"
#include "Component/SkyComponent.h"
#include "RenderPass/ERPPass.h"

SkyLightActor::SkyLightActor()
{
	m_SkyComponent = AddComponent<SkyComponent>();
	SetRootComponent(m_SkyComponent);
}

void SkyLightActor::OnSpawn()
{
	m_ERPPass = CreateRef<ERPPass>("Assets/Textures/newport_loft.hdr", 512);
	m_SkyComponent->SetCubeMapTexture(m_ERPPass->GetCubeMapTexture());
}

void SkyLightActor::SetCubeMapTexture(Ref<TextureCube> cubeMapTexture) const
{
	m_SkyComponent->SetCubeMapTexture(std::move(cubeMapTexture));
}
