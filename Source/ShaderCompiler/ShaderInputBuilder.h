#pragma once

#include <string>
#include <vector>

#include "Common/Material/MaterialBuilder.h"
#include "Common/Material/MaterialCommon.h"
#include "Common/Material/MaterialTypes.h"
#include "RHI/VertexBuffer.h"

// =============================================================================
// Structured input builders.
// =============================================================================

/// POSITION (location=0) is always included for surface domain.
std::vector<VariableParam> BuildVertexInputs(MaterialDomain domain,
	const RHI::AttributeBitset& requiredAttributes);

/// Default fragColor (location=0, float4) + custom outputs.
std::vector<VariableParam> BuildFragmentOutputs(
	const MaterialBuilder::OutputList& outputs);

// ── Helpers ─────────────────────────────────────────────────────────────────

FieldType GLSLTypeToFieldType(const char* glslType);
