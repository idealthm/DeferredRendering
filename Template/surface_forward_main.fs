
layout(location = 0) out vec4 fragColor;

void main() {
    // filament_lodBias = frameUniforms.lodBias;

    // initObjectUniforms();

    // Initialize the inputs to sensible default values, see surface_material_inputs.fs
    MaterialInputs inputs;
    initMaterial(inputs);

    // Invoke user code
    material(inputs);

    fragColor = evaluateMaterial(inputs);
}
