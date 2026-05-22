#pragma once

#include <cstdint>

#include "RHI/DriverEnums.h"


enum class DescriptorSetBindingPoints : uint8_t {
	PER_VIEW        = 0,
	PER_RENDERABLE  = 1,
	PER_MATERIAL    = 2,
	G_BUFFER		= 3,
};


enum class PerViewBindingPoint : uint8_t {
	FRAME_UNIFORM = 0,
};

enum class PerRenderableBindingPoint : uint8_t {
	OBJECT_UNIFORM = 0,
};

enum class PerMaterialBindingPoint : uint8_t {
	MATERIAL_UNIFORM = 0,
};

enum class GBufferBindingPoint : uint8_t {
	G_BUFFER_ALBEDO = 0,
	G_BUFFER_NORMAL = 1,
	G_BUFFER_POSITION = 2,
	G_BUFFER_MATERIAL = 3,
	G_BUFFER_DEPTH = 4,
};

template <> struct EnableBitMaskOperators<DescriptorSetBindingPoints> : public std::true_type {};
template <> struct EnableIntegerOperators<DescriptorSetBindingPoints> : public std::true_type {};
template <> struct EnableBitMaskOperators<PerViewBindingPoint> : public std::true_type {};
template <> struct EnableIntegerOperators<PerViewBindingPoint> : public std::true_type {};
template <> struct EnableBitMaskOperators<PerRenderableBindingPoint> : public std::true_type {};
template <> struct EnableIntegerOperators<PerRenderableBindingPoint> : public std::true_type {};
template <> struct EnableBitMaskOperators<PerMaterialBindingPoint> : public std::true_type {};
template <> struct EnableIntegerOperators<PerMaterialBindingPoint> : public std::true_type {};
template <> struct EnableBitMaskOperators<GBufferBindingPoint> : public std::true_type {};
template <> struct EnableIntegerOperators<GBufferBindingPoint> : public std::true_type {};