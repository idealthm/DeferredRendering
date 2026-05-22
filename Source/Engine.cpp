#include "Engine.h"

#include "Material/MaterialLibrary.h"
#include "RHI/GL/GLDriver.h"

Engine* gEngine = nullptr;

void Engine::Init()
{
	m_Driver = std::make_unique<RHI::GLDriver>();
	m_Driver->Init();
	m_ModelMaterial = MaterialLibrary::Get().GetMaterial("Model");
	m_ShapeMaterial = MaterialLibrary::Get().GetMaterial("Shape");
}

void Engine::Shutdown()
{
	m_Driver.reset();
}
