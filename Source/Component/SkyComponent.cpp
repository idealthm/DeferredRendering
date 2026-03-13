#include "SkyComponent.h"

#include "Shader/Shader.h"

void SkyComponent::SetCubeMapTexture(Ref<TextureCube> cubeMapTexture)
{
	m_CubeMap = cubeMapTexture;
}
