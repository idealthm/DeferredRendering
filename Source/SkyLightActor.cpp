#include "SkyLightActor.h"

#include "RenderPipeline.h"
#include "Component/SkyComponent.h"
#include "RenderPass/EnvPreFilter.h"
#include "RenderPass/ERPPass.h"

SkyLightActor::SkyLightActor()
{
	m_SkyComponent = AddComponent<SkyComponent>();
	SetRootComponent(m_SkyComponent);
}

void SkyLightActor::OnSpawn()
{
}

