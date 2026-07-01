#pragma once

#include <cstdint>

#include "RHI/DriverEnums.h"


enum class DescriptorSetBindingPoints : uint8_t {
	PER_VIEW        = 0,
	PER_RENDERABLE  = 1,
	PER_MATERIAL    = 2,
	G_BUFFER		= 3,
};


enum class PerViewBindingPoints : uint8_t {
	FRAME_UNIFORM  = 0,
	LIGHT_DATA     = 1,
	IBL_IRRADIANCE = 2,
	IBL_PREFILTER  = 3,
	BRDF_LUT       = 4,
	NUM_PER_VIEW_BINDING_POINTS,
};

enum class PerRenderableBindingPoints : uint8_t {
	OBJECT_UNIFORM = 0,
	NUM_PER_RENDERABLE_BINDING_POINTS,
};

enum class PerMaterialBindingPoint : uint8_t {
	MATERIAL_UNIFORM = 0,
};

enum class GBufferBindingPoint : uint8_t {
	G_BUFFER_DEPTH    = 0,
	G_BUFFER_NORMAL   = 1,
	G_BUFFER_ALBEDO   = 2,
	G_BUFFER_MATERIAL = 3,
};

template <> struct EnableBitMaskOperators<DescriptorSetBindingPoints> : public std::true_type {};
template <> struct EnableIntegerOperators<DescriptorSetBindingPoints> : public std::true_type {};
template <> struct EnableBitMaskOperators<PerViewBindingPoints> : public std::true_type {};
template <> struct EnableIntegerOperators<PerViewBindingPoints> : public std::true_type {};
template <> struct EnableBitMaskOperators<PerRenderableBindingPoints> : public std::true_type {};
template <> struct EnableIntegerOperators<PerRenderableBindingPoints> : public std::true_type {};
template <> struct EnableBitMaskOperators<PerMaterialBindingPoint> : public std::true_type {};
template <> struct EnableIntegerOperators<PerMaterialBindingPoint> : public std::true_type {};
template <> struct EnableBitMaskOperators<GBufferBindingPoint> : public std::true_type {};
template <> struct EnableIntegerOperators<GBufferBindingPoint> : public std::true_type {};