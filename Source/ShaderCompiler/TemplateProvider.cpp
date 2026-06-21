#include "TemplateProvider.h"

namespace Provider {
namespace {
	std::vector<std::string> s_CommonProlog = {"common_defines.glsl","common_getter.glsl","common_math.glsl"};
	std::vector<std::string> s_DepthVertex   = {"surface_instancing.glsl","surface_getters.vs","surface_material_input.vs"};
	std::vector<std::string> s_DepthFragment = {};
	std::vector<std::string> s_GBufferVertex = {"surface_instancing.glsl","surface_getters.vs","surface_material_input.vs"};
	std::vector<std::string> s_GBufferFragment = {"common_shading.glsl","surface_instancing.glsl","surface_material_input.fs","surface_getters.fs","surface_shading_parameters.fs"};
	std::vector<std::string> s_LightingVertex = {};
	std::vector<std::string> s_LightingFragment = {"common_shading.glsl","surface_material_input.fs","lighting_uniforms.glsl","surface_shading_parameters.fs","surface_shading_unlit.fs","surface_shading_lit.fs"};
	std::vector<std::string> s_PostProcessVertex = {"post_process_inputs.vs","post_process_getters.vs"};
	std::vector<std::string> s_PostProcessFragment = {"post_process_inputs.fs"};
}
const std::vector<std::string>& GetCommonProlog() { return s_CommonProlog; }
const std::vector<std::string>& GetDepthVertex()    { return s_DepthVertex; }
const std::vector<std::string>& GetDepthFragment()  { return s_DepthFragment; }
const std::vector<std::string>& GetGBufferVertex()  { return s_GBufferVertex; }
const std::vector<std::string>& GetGBufferFragment(){ return s_GBufferFragment; }
const std::vector<std::string>& GetLightingVertex() { return s_LightingVertex; }
const std::vector<std::string>& GetLightingFragment(){ return s_LightingFragment; }
const std::vector<std::string>& GetPostProcessVertex() { return s_PostProcessVertex; }
const std::vector<std::string>& GetPostProcessFragment() { return s_PostProcessFragment; }
}
