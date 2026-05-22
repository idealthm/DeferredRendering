#pragma once

#include <memory>

#include "RHI/RHIDriver.h"

class Material;

class Engine
{
public:
	Engine() = default;

	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;

	void Init();
	void Shutdown();

	RHI::RHIDriver& GetDriver() { return *m_Driver; }
	Ref<Material> GetDefaultModelMaterial() { return m_ModelMaterial; }
	Ref<Material> GetDefaultShapeMaterial() { return m_ShapeMaterial; }

private:
	std::unique_ptr<RHI::RHIDriver> m_Driver;
	Ref<Material> m_ModelMaterial;
	Ref<Material> m_ShapeMaterial;
};

extern Engine* gEngine;
