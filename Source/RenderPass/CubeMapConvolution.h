#pragma once
#include "Shapes/ScreenQuad.h"

class MaterialInstance;
class RenderTarget;
class Texture;

class CubeMapConvolution
{
public:
	CubeMapConvolution() = default;

	/// Irradiance convolution — single cubemap face iteration
	void RenderIrradiance(Ref<Texture>& kernelTexture, Ref<Texture>& srcCubeMap,
		Ref<Texture>& dstCubeMap, uint32_t sampleCount);

	/// Pre-filter envmap convolution — one roughness level
	void RenderPrefilter(Ref<Texture>& kernelTexture, Ref<Texture>& srcCubeMap,
		Ref<Texture>& dstCubeMap, uint32_t mipLevel, uint32_t sampleCount);

private:
	void RenderCubemapFaces(Ref<MaterialInstance>& mi, Ref<Texture>& srcCubeMap,
		Ref<Texture>& kernelTex, Ref<Texture>& dstCubeMap,
		uint32_t mipLevel, uint32_t sampleCount);
};
