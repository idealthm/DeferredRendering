#pragma once
#include "BufferInterfaceBlock.h"
#include "SamplerInterfaceBlock.h"
#include "RHI/VertexBuffer.h"


struct MaterialInfo {
	bool isLit;
	bool hasDoubleSidedCapability;
	bool has3dSamplers;
	bool flipUV;
	RHI::AttributeBitset requiredAttributes;
	BlendingMode blendingMode;
	BlendingMode postLightingBlendingMode;
	Shading shading;
	BufferInterfaceBlock uib;
	SamplerInterfaceBlock sib;
	glm::uvec3 groupSize;

	using BufferContainer = std::vector<BufferInterfaceBlock const*>;
	BufferContainer buffers;
};
