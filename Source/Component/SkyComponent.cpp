#include "SkyComponent.h"

#include "Shader/Program.h"

void SkyComponent::SetCubeMapTexture(Ref<TextureCube> cubeMapTexture)
{
	m_CubeMap = cubeMapTexture;
}
