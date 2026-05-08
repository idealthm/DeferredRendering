
layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gAlbedo;
layout(location = 3) out vec3 gNormal;
layout(location = 2) out vec3 gRMS;

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

    gAlbedo = inputs.baseColor;

    gRMS = vec4(inputs.roughness, inputs.metallic, inputs.ambientOcclusion);

    gNormal = inputs.normal;
}
