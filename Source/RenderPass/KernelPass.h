#pragma once
#include "Shapes/ScreenQuad.h"

class MaterialInstance;
class RenderTarget;
class Texture;

class KernelPass
{
public:
	KernelPass() = default;

	/// Generate kernel texture (numLevels rows x sampleCount columns, RGBA16F).
	/// @param numLevels   number of roughness levels (rows), 1 for irradiance
	/// @param sampleCount samples per level (columns)
	/// @param irradiance  true → cosine-weight; false → GGX importance sample
	/// @param outKernel   output kernel texture
	void Generate(uint32_t numLevels, uint32_t sampleCount, bool irradiance, Ref<Texture>& outKernel);
};
