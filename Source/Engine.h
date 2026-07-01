#pragma once

#include <memory>

#include "Material/LightingShaderLibrary.h"
#include "RHI/RHIDriver.h"

class Material;
class ScreenQuad;

class Engine
{
public:
	Engine() = default;

	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;

	void Init();
	void Shutdown();

	RHI::RHIDriver& GetDriver() { return *m_Driver; }
	const Ref<Material>& GetDefaultModelMaterial() { return m_ModelMaterial; }
	const Ref<Material>& GetDefaultShapeMaterial() { return m_ShapeMaterial; }

	DescriptorSetLayout const& GetPerRenderableSetLayout();
	DescriptorSetLayout const& GetPerViewSetLayout();
	DescriptorSetLayout const& GetGBufferSetLayout();

	LightingShaderLibrary& GetLightingShaderLibrary() { return m_LightingShaderLibrary; }

	ScreenQuad& GetScreenQuad() const { return *m_ScreenQuad; }

private:
	std::unique_ptr<RHI::RHIDriver> m_Driver;
	Ref<Material> m_ModelMaterial;
	Ref<Material> m_ShapeMaterial;

	DescriptorSetLayout m_PerRenderableSetLayout;
	DescriptorSetLayout m_PerViewSetLayout;
	DescriptorSetLayout m_GBufferSetLayout;
	LightingShaderLibrary m_LightingShaderLibrary;

	Ref<ScreenQuad> m_ScreenQuad;
};

extern Engine* gEngine;
