#pragma once
#include <cstdint>

#include "DriverEnums.h"
#include "Common/Handle.h"

namespace RHI
{
	struct HwProgram;

struct PipelineLayout {
	using SetLayout = std::array<Handle<HwDescriptorSetLayout>, MAX_DESCRIPTOR_SET_COUNT>;
	SetLayout setLayout;      // 16
};

struct PipelineState {
	Handle<HwProgram> program;                                         //  4
	Handle<HwVertexBufferInfo> vertexBufferInfo;                       //  4
	PipelineLayout pipelineLayout;                                          // 16
	RasterState rasterState;                                           //  4
	StencilState stencilState;                                         // 12
	PolygonOffset polygonOffset;                                       //  8
	PrimitiveType primitiveType = PrimitiveType::TRIANGLES;       //  1
	uint8_t padding[3] = {};                                                //  3
};

}
