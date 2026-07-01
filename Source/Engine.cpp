#include "Engine.h"

#include "DescriptorSets.h"
#include "Material/MaterialLibrary.h"
#include "RHI/GL/GLDriver.h"
#include "Shapes/ScreenQuad.h"


Engine* gEngine = nullptr;

void Engine::Init()
{
	m_Driver = std::make_unique<RHI::GLDriver>();
	m_Driver->Init();
	m_ModelMaterial = MaterialLibrary::Get().GetMaterial("Model");
	m_ShapeMaterial = MaterialLibrary::Get().GetMaterial("Shape");

	m_LightingShaderLibrary.Init("CompiledMaterials");

	m_PerRenderableSetLayout = DescriptorSetLayout(*m_Driver, DescriptorSets::GetPerRenderableSetLayout());
	m_PerViewSetLayout = DescriptorSetLayout(*m_Driver, DescriptorSets::GetPerViewSetLayout());
	m_GBufferSetLayout = DescriptorSetLayout(*m_Driver, DescriptorSets::GetGBufferSetLayout());

	m_ScreenQuad = CreateRef<ScreenQuad>();
}

void Engine::Shutdown()
{
	m_Driver.reset();
}

DescriptorSetLayout const& Engine::GetPerRenderableSetLayout()
{
	return m_PerRenderableSetLayout;
}

DescriptorSetLayout const& Engine::GetPerViewSetLayout()
{
	return m_PerViewSetLayout;
}

DescriptorSetLayout const& Engine::GetGBufferSetLayout()
{
	return m_GBufferSetLayout;
}
