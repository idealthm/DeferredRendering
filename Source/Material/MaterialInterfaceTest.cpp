// Integration test: Material + ShaderParser + MaterialInstance (single MaterialParams UBO)
//
// Only MaterialParams UBO + material samplers belong to Material.
// Engine uniforms (FrameUniforms etc.) are filtered out.
// UniformBuffer = CPU data + typed access; Buffer = Handle<HwBuffer> wrapper.

#include "Buffer/Buffer.h"
#include "Material/MaterialInstance.h"
#include "SpirvReflect/ShaderParse.h"
#include "UnifromBuffer/UniformBuffer.h"

#include <cassert>
#include <cstdio>
#include <cstring>

static int g_Failures = 0;
#define CHECK(cond) do { if (!(cond)) { \
	printf("  FAIL [%d]: %s\n", __LINE__, #cond); \
	g_Failures++; \
} } while(0)

static const char* kFragSpv = "Material/CompiledMaterials/lit/lit.frag.spv";
static const char* kVertSpv = "Material/CompiledMaterials/lit/lit.vert.spv";

// ── Parse fragment shader ──────────────────────────────────────────────────
static void TestParseFragmentShader()
{
	printf("Test: Parse fragment shader...\n");

	ShaderParser parser("", kFragSpv);
	CHECK(parser.IsValid());

	const auto& info = parser.GetMaterialInfo();
	const auto& matBlock = info.uib;

	CHECK(!matBlock.structName.empty());
	CHECK(matBlock.size == 80);
	CHECK(matBlock.fields.size() == 3);

	bool okB = false, okR = false, okX = false;
	for (const auto& f : matBlock.fields)
	{
		if (f.name == "baseColor")         { CHECK(f.type == FieldType::FLOAT3); CHECK(f.offset == 0);  okB = true; }
		if (f.name == "roughness")         { CHECK(f.type == FieldType::FLOAT);  CHECK(f.offset == 12); okR = true; }
		if (f.name == "textureTransform")  { CHECK(f.type == FieldType::MAT4);   CHECK(f.offset == 16); okX = true; }
	}
	CHECK(okB && okR && okX);
}

// ── Fragment sampler ───────────────────────────────────────────────────────
static void TestFragmentSamplers()
{
	printf("Test: Fragment shader samplers...\n");

	ShaderParser parser("", kFragSpv);
	CHECK(parser.IsValid());

	const auto& info = parser.GetMaterialInfo();
	const SamplerInfo* tex = nullptr;
	for (const auto& s : info.sib.mSamplersInfoList)
	{
		if (s.name.empty()) break;
		if (s.name == "materialParams_texture") { tex = &s; break; }
	}

	CHECK(tex != nullptr);
	CHECK(tex->binding == 4);
	CHECK(tex->sampler == RHI::Sampler::Dim2D);
}

// ── Vertex shader I/O ──────────────────────────────────────────────────────
static void TestVertexShaderIO()
{
	printf("Test: Vertex shader inputs/outputs...\n");

	ShaderParser parser(kVertSpv, "");
	CHECK(parser.IsValid());

	const auto& info = parser.GetMaterialInfo();

	bool pos = false, tan = false;
	for (const auto& v : info.inputVariables)
	{
		if (v.name == "mesh_position")  { CHECK(v.location == 0); pos = true; }
		if (v.name == "mesh_tangents")  { CHECK(v.location == 1); tan = true; }
	}
	CHECK(pos && tan);
}

// ── Build Material from fragment shader (only MaterialParams + samplers) ───
static void TestBuildMaterialFromShader()
{
	printf("Test: Build Material from shader...\n");

	ShaderParser parser("", kFragSpv);
	CHECK(parser.IsValid());

	Material mat;
	mat.BuildFromReflection(parser.GetMaterialInfo());

	// Only MaterialParams is loaded
	const auto& block = mat.GetUniformBlock();
	CHECK(block.structName == "MaterialParams");
		CHECK(block.fields.size() == 3);

	// Field lookup (flat, single block)
	const auto* roughness = mat.FindField("roughness");
	CHECK(roughness != nullptr);
	CHECK(roughness->type == FieldType::FLOAT);
	CHECK(roughness->offset == 12);

	const auto* baseColor = mat.FindField("baseColor");
	CHECK(baseColor != nullptr);
	CHECK(baseColor->type == FieldType::FLOAT3);

	CHECK(mat.FindField("nonexistent") == nullptr);
	CHECK(mat.FindField("lodBias") == nullptr);  // FrameUniforms — filtered out

	// Sampler
	const auto* tex = mat.FindSampler("materialParams_texture");
	CHECK(tex != nullptr);
	CHECK(tex->binding == 4);

	// Engine uniforms NOT in Material
	CHECK(mat.FindSampler("frameUniforms") == nullptr);
}

// ── Material field lookup ──────────────────────────────────────────────────
static void TestMaterialFieldLookup()
{
	printf("Test: Material field lookup...\n");

	ShaderParser parser("", kFragSpv);
	CHECK(parser.IsValid());

	Material mat;
	mat.BuildFromReflection(parser.GetMaterialInfo());

	const auto* f0 = mat.FindField("baseColor");
	CHECK(f0 != nullptr && f0->type == FieldType::FLOAT3);

	const auto* f1 = mat.FindField("roughness");
	CHECK(f1 != nullptr && f1->type == FieldType::FLOAT);

	CHECK(mat.FindField("nonexistent") == nullptr);
	CHECK(mat.GetFields().size() == 3);
}

// ── UniformBuffer typed Set/Get ────────────────────────────────────────────
static void TestUniformBufferSetGet()
{
	printf("Test: UniformBuffer typed Set/Get...\n");

	ShaderParser parser("", kFragSpv);
	CHECK(parser.IsValid());

	Material mat;
	mat.BuildFromReflection(parser.GetMaterialInfo());

	UniformBuffer ub(mat.GetUniformBlock());
	ub.Clear();

	// Set roughness (float at offset 12)
	float roughness = 0.6f;
	CHECK(ub.SetValue("roughness", roughness));
	CHECK(ub.IsDirty());

	float outR = -1.0f;
	CHECK(ub.GetValue("roughness", outR));
	CHECK(outR == 0.6f);

	// Set baseColor (vec3 at offset 0)
	glm::vec3 color(0.8f, 0.4f, 0.2f);
	CHECK(ub.SetValue("baseColor", color));

	glm::vec3 outC(0.0f);
	CHECK(ub.GetValue("baseColor", outC));
	CHECK(outC == color);

	// Set textureTransform (mat4 at offset 16)
	glm::mat4 xform(1.0f);
	xform[3][1] = 3.0f;
	CHECK(ub.SetValue("textureTransform", xform));

	glm::mat4 outX(0.0f);
	CHECK(ub.GetValue("textureTransform", outX));
	CHECK(outX[3][1] == 3.0f);

	// Verify raw bytes via GetData
	const uint8_t* data = ub.GetData();
	float rawR;
	std::memcpy(&rawR, data + 12, sizeof(float));
	CHECK(rawR == 0.6f);

	glm::mat4 rawX;
	std::memcpy(&rawX, data + 16, sizeof(glm::mat4));
	CHECK(rawX[3][1] == 3.0f);

	// Wrong field type rejected
	double bad = 1.0;
	CHECK(!ub.SetValue("roughness", bad));

	// Nonexistent field
	CHECK(!ub.SetValue("nonexistent", 1.0f));
}

// ── Buffer handle wrapper ──────────────────────────────────────────────────
static void TestBufferHandle()
{
	printf("Test: Buffer handle wrapper...\n");

	// Default construction — null handle
	Buffer buf;
	CHECK(!buf.IsValid());
	CHECK(buf.GetHandleId() == HandleBase::NullId);

	// Set a handle
	Handle<HwBuffer> h(42u);
	CHECK(h.GetId() == 42u);

	buf.SetHandle(std::move(h));
	CHECK(buf.IsValid());
	CHECK(buf.GetHandleId() == 42u);
	CHECK(!h);  // moved-from handle is null

	// Move assignment
	Buffer buf2;
	buf2 = std::move(buf);
	CHECK(buf2.IsValid());
	CHECK(buf2.GetHandleId() == 42u);
	CHECK(!buf.IsValid());  // moved-from Buffer has null handle
}

// ── MaterialInstance via descriptor set ─────────────────────────────────────
static void TestMaterialInstanceSetGet()
{
	printf("Test: MaterialInstance via descriptor set...\n");

	ShaderParser parser("", kFragSpv);
	CHECK(parser.IsValid());

	Material mat;
	mat.BuildFromReflection(parser.GetMaterialInfo());

	MaterialInstance mi(&mat);
	mi.Init();

	// Set roughness
	float roughness = 0.3f;
	CHECK(mi.SetParameter("roughness", roughness));
	CHECK(mi.IsDirty());

	float outR = 0.0f;
	CHECK(mi.GetParameter("roughness", outR));
	CHECK(outR == 0.3f);

	// Set baseColor
	glm::vec3 color(0.1f, 0.2f, 0.3f);
	CHECK(mi.SetParameter("baseColor", color));

	glm::vec3 outC(0.0f);
	CHECK(mi.GetParameter("baseColor", outC));
	CHECK(outC == color);

	// Verify raw data via UniformBuffer (not Buffer handle)
	const uint8_t* data = mi.GetUniformData();
	CHECK(data != nullptr);
	float rawR;
	std::memcpy(&rawR, data + 12, sizeof(float));
	CHECK(rawR == 0.3f);
}

// ── Descriptor set structure ───────────────────────────────────────────────
static void TestDescriptorSetInstance()
{
	printf("Test: DescriptorSetInstance structure...\n");

	ShaderParser parser("", kFragSpv);
	CHECK(parser.IsValid());

	Material mat;
	mat.BuildFromReflection(parser.GetMaterialInfo());

	MaterialInstance mi(&mat);
	mi.Init();

	const auto& ds = mi.GetDescriptorSet();
	CHECK(ds.set == 0);

	// Buffer slot at binding 0 — handle is null until engine sets it
	CHECK(ds.HasBinding(0));
	CHECK(!ds.GetBufferHandle(0));
	CHECK(mi.GetUniformDataSize() == 80);

	// Sampler slot at binding 4 — texture handle is null until engine sets it
	CHECK(ds.HasBinding(4));
	CHECK(!ds.GetTextureHandle(4));

	// Not active
	CHECK(!ds.HasBinding(1));   // FrameUniforms — not in Material
	CHECK(!ds.HasBinding(5));
}

// ── Dirty tracking ─────────────────────────────────────────────────────────
static void TestDirtyFlag()
{
	printf("Test: Dirty tracking...\n");

	ShaderParser parser("", kFragSpv);
	CHECK(parser.IsValid());

	Material mat;
	mat.BuildFromReflection(parser.GetMaterialInfo());

	MaterialInstance mi(&mat);
	mi.Init();
	CHECK(mi.IsDirty());

	mi.ClearDirty();
	CHECK(!mi.IsDirty());

	mi.SetParameter("roughness", 0.5f);
	CHECK(mi.IsDirty());

	mi.ClearDirty();
	CHECK(!mi.IsDirty());
}

// ── Wrong field name ───────────────────────────────────────────────────────
static void TestWrongField()
{
	printf("Test: Wrong field name...\n");

	ShaderParser parser("", kFragSpv);
	CHECK(parser.IsValid());

	Material mat;
	mat.BuildFromReflection(parser.GetMaterialInfo());

	MaterialInstance mi(&mat);
	mi.Init();

	CHECK(!mi.SetParameter("nonexistent", 1.0f));

	float out;
	CHECK(!mi.GetParameter("nonexistent", out));
}

// ── Unbound / re-bind ──────────────────────────────────────────────────────
static void TestUnboundAndRebind()
{
	printf("Test: Unbound / re-bind...\n");

	MaterialInstance mi;
	CHECK(!mi.SetParameter("anything", 1.0f));
	CHECK(!mi.IsAllocated());
	CHECK(!mi.GetDescriptorSet().activeBindings.any());

	ShaderParser parser("", kFragSpv);
	CHECK(parser.IsValid());
	Material mat;
	mat.BuildFromReflection(parser.GetMaterialInfo());

	mi.BindMaterial(&mat);
	mi.Init();
	CHECK(mi.IsAllocated());
	CHECK(mi.SetParameter("roughness", 0.7f));

	mi.BindMaterial(&mat);
	CHECK(!mi.IsAllocated());
}

// ── GetUniformData ─────────────────────────────────────────────────────────
static void TestGetUniformData()
{
	printf("Test: GetUniformData...\n");

	ShaderParser parser("", kFragSpv);
	CHECK(parser.IsValid());

	Material mat;
	mat.BuildFromReflection(parser.GetMaterialInfo());

	MaterialInstance mi(&mat);
	mi.Init();

	CHECK(mi.GetUniformData() != nullptr);
	CHECK(mi.GetUniformDataSize() == 80);
}

// ── Defaults are zero ──────────────────────────────────────────────────────
static void TestDefaultsAreZero()
{
	printf("Test: Defaults are zero...\n");

	ShaderParser parser("", kFragSpv);
	CHECK(parser.IsValid());

	Material mat;
	mat.BuildFromReflection(parser.GetMaterialInfo());

	MaterialInstance mi(&mat);
	mi.Init();

	const uint8_t* data = mi.GetUniformData();
	for (uint32_t i = 0; i < mi.GetUniformDataSize(); i++)
		CHECK(data[i] == 0);
}

// ── Sampler slot ───────────────────────────────────────────────────────────
static void TestSamplerSlot()
{
	printf("Test: Sampler slot value...\n");

	ShaderParser parser("", kFragSpv);
	CHECK(parser.IsValid());

	Material mat;
	mat.BuildFromReflection(parser.GetMaterialInfo());

	MaterialInstance mi(&mat);
	mi.Init();

	const auto& ds = mi.GetDescriptorSet();
	CHECK(ds.HasBinding(4));
	CHECK(!ds.GetTextureHandle(4));  // engine sets texture handle

	// Sampler type lives in Material definition, not descriptor set
	CHECK(mat.FindSampler("materialParams_texture")->sampler == RHI::Sampler::Dim2D);
}

// ── Wrong type rejected ────────────────────────────────────────────────────
static void TestWrongType()
{
	printf("Test: Wrong type rejected by MaterialInstance...\n");

	ShaderParser parser("", kFragSpv);
	CHECK(parser.IsValid());

	Material mat;
	mat.BuildFromReflection(parser.GetMaterialInfo());

	MaterialInstance mi(&mat);
	mi.Init();

	double bad = 1.0;
	CHECK(!mi.SetParameter("roughness", bad));
}

int main()
{
	printf("=== Material + Buffer Integration Tests ===\n\n");

	TestParseFragmentShader();
	TestFragmentSamplers();
	TestVertexShaderIO();
	TestBuildMaterialFromShader();
	TestMaterialFieldLookup();
	TestUniformBufferSetGet();
	TestBufferHandle();
	TestMaterialInstanceSetGet();
	TestDescriptorSetInstance();
	TestDirtyFlag();
	TestWrongField();
	TestWrongType();
	TestUnboundAndRebind();
	TestGetUniformData();
	TestDefaultsAreZero();
	TestSamplerSlot();

	printf("\n=== Results: %d failure(s) ===\n", g_Failures);
	return g_Failures;
}
