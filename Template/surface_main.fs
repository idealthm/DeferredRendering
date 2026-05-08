
layout(location = 0) out vec4 fragColor;

void main() {
    // filament_lodBias = frameUniforms.lodBias;

    // initObjectUniforms();

    // See surface_shading_parameters.fs
    // Computes global variables we need to evaluate material and lighting
    // computeShadingParams();

    // Initialize the inputs to sensible default values, see surface_material_inputs.fs
    MaterialInputs inputs;
    initMaterial(inputs);

    // Invoke user code
    material(inputs);

    // applyAlphaMask(inputs.baseColor);

    fragColor = evaluateMaterial(inputs);
}
