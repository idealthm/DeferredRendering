#pragma once

#include <string>
#include <vector>

#include "Common/Material/MaterialCommon.h"
#include "Common/Material/MaterialTypes.h"

struct MaterialSpec;

// =============================================================================
// Structured input builders — produce std::vector<VariableParam> from
// MaterialSpec.  Used by both GLSL generation and chunk metadata generation.
// =============================================================================

/// Vertex inputs from material's required attributes.
/// POSITION (location=0) is always included for surface domain.
std::vector<VariableParam> BuildVertexInputs(const MaterialSpec& spec);

/// Fragment outputs — default fragColor (location=0) + spec.outputs.
std::vector<VariableParam> BuildFragmentOutputs(const MaterialSpec& spec);

// ── Helpers ─────────────────────────────────────────────────────────────────

/// GLSL type string → FieldType
FieldType GLSLTypeToFieldType(const char* glslType);
