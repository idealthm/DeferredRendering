#pragma once
#include <array>
#include <glm/glm.hpp>

#include "Lights/Light.h"

constexpr size_t CONFIG_MAX_INSTANCES = 64;


struct PerViewUib
{
	static constexpr std::string_view _name = std::string_view{"FrameUniforms"};

	glm::mat4 viewFromWorldMatrix;	// clip		view  <-	world
	glm::mat4 worldFromViewMatrix;	// clip		view  ->	world
	glm::mat4 clipFromViewMatrix; 	// clip	<-	view	 	world
	glm::mat4 viewFromClipMatrix; 	// clip	->	view	 	world
	glm::mat4 clipFromWorldMatrix;	// clip	<-	view  <- 	world
	glm::mat4 worldFromClipMatrix;	// clip	->	view  -> 	world

	int RenderMode;
	int _pad0[3];               // std140 padding to align iblParams at offset 400
	alignas(16) glm::vec4 iblParams; // x=prefilterMipCount, y=skyboxIntensity, z=envIntensity
};

struct alignas(16) LightInfo {
	glm::vec3 position;   // w: 影响半径 (Range / Attenuation Radius)
	float range;

	glm::vec3 color;     // w: 强度 (Intensity)
	float intensity;

	glm::vec3 direction; // w: 光源类型 (0: 方向光, 1: 点光源, 2: 聚光灯)
	int32_t type;

	glm::vec4 params;    // x: 聚光灯内角, y: 聚光灯外角, z: 是否产生阴影, w: 预留
};

// GLSL counterpart: vec4 position; vec4 color; vec4 direction; vec4 params;
static_assert(sizeof(LightInfo) == 64, "LightInfo must be 64 bytes (4 x vec4 in std140)");

struct LightData
{
	LightInfo lights[16];
	glm::mat4 uLightVP;
	int32_t NumLights;
};

namespace std140
{
	
struct alignas(16) vec3 : public std::array<float, 3> {};
struct alignas(16) vec4 : public std::array<float, 4> {};

struct mat33 : public std::array<vec3, 3> {
	mat33& operator=(glm::mat3 const& rhs) noexcept {
		for (int i = 0; i < 3; i++) {
			(*this)[i][0] = rhs[i][0];
			(*this)[i][1] = rhs[i][1];
			(*this)[i][2] = rhs[i][2];
		}
		return *this;
	}
};

struct mat44 : public std::array<vec4, 4> {
	mat44& operator=(glm::mat4 const& rhs) noexcept {
		for (int i = 0; i < 4; i++) {
			(*this)[i][0] = rhs[i][0];
			(*this)[i][1] = rhs[i][1];
			(*this)[i][2] = rhs[i][2];
			(*this)[i][3] = rhs[i][3];
		}
		return *this;
	}
};

}

struct PerRenderableData
{
	std140::mat44 worldFromModelMatrix;
	std140::mat33 worldFormModelNormalMatrix;
	int32_t morphTargetCount;
	int32_t flagsChannels;                   // see packFlags() below (0x00000fll)
	int32_t objectId;                        // used for picking
	float userData;

	glm::vec4 reserved[8];
};

static_assert(sizeof(PerRenderableData) == 256, "PerRenderableData size must be 256 bytes");

struct PerRenderableUib
{
	static constexpr std::string_view _name = std::string_view{"ObjectUniforms"};
	PerRenderableData models[CONFIG_MAX_INSTANCES];
};
