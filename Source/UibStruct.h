#pragma once
#include <array>
#include <glm/glm.hpp>

#include "Lights/Light.h"

constexpr size_t CONFIG_MAX_INSTANCES = 128;


struct PerViewUib
{
	glm::mat4 m_viewFormWorldMatrix;	// clip		view  <-	world
	glm::mat4 m_worldFormViewMatrix;	// clip		view  ->	world
	glm::mat4 m_clipFromViewMatrix; 	// clip	<-	view	 	world
	glm::mat4 m_viewFromClipMatrix; 	// clip	->	view	 	world
	glm::mat4 m_clipFromWorldMatrix;	// clip	<-	view  <- 	world
	glm::mat4 m_worldFromClipMatrix;	// clip	->	view  -> 	world

	int RenderMode;
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

struct ModelInfo
{
	std140::mat44 ModelTransform;
	std140::mat33 ModelNormal;
};

struct ModelData
{
	ModelInfo models[CONFIG_MAX_INSTANCES];
};
