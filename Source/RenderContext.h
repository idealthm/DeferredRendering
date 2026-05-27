#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "Common/Handle.h"
#include "UibStruct.h"
#include "UnifromBuffer/ParamBuffer.h"

namespace RHI { struct HwRenderTarget; }

class Texture;

struct RenderContext
{
	Handle<RHI::HwRenderTarget> GBufferRT;
	uint32_t renderMode;
	uint32_t usedTextureSlot;

	glm::u32vec2 viewportSize = {1600, 900};
	float ShadowWidth, ShadowHeight;

	Ref<Texture> BRDF_LUT;

	Ref<Texture> GBuffer_Position;
	Ref<Texture> GBuffer_Normal;
	Ref<Texture> GBuffer_Albedo;
	Ref<Texture> GBuffer_Material;
	Ref<Texture> GBuffer_Depth;

	Ref<Texture> Test;

	Ref<Texture> ShadowMap_Depth;

	Ref<Texture> LightMap_SceneColor;
	Ref<Texture> LightMap_SceneDepth;

	Ref<Texture> ERP_Cubemap;
	Ref<Texture> IBL_IrradianceMap;
	Ref<Texture> IBL_PreFilterMap;

	Ref<Texture> Sky_SceneColor;

	Ref<Texture> Final_SceneColor;

	TypedUniformBuffer<PerViewUib> FrameDataUB;
	TypedUniformBuffer<LightData> LightDataUB;
	TypedUniformBuffer<ModelData> ModelDataUB;

	Handle<RHI::HwBufferObject> FrameDataHandle;
	Handle<RHI::HwBufferObject> LightDataHandle;
	Handle<RHI::HwBufferObject> ModelDataHandle;
};
