#include "LightingPass.h"

#include <glm/detail/type_quat.hpp>

#include "GBufferPass.h"
#include "Scene.h"
#include "FrameBuffer/FrameBuffer.h"
#include "Lights/Light.h"
#include "Model/Texture.h"
#include "Shader/Shader.h"
#include "Shapes/MeshBuilder.h"
#include "glad/glad.h"
#include "Shader/ShaderLibrary.h"

LightingPass::LightingPass(uint32 width, uint32 height)
	: RenderPass(width, height)
{
	m_Shader = ShaderLibrary::Get().GetShader("Shaders/DirectionLight", nullptr);
}

/*void LightingPass::Setup(RenderContext& ctx, FBAttachmentInfo & info)
{
	info.Depth = {&ctx.LightMap_SceneDepth, EFBTextureFormat::Depth, FBTextureLoadAction::Clear, FBTextureStoreAction::Store}; // Depth
	info.Attachments = {
		{&ctx.LightMap_SceneColor, EFBTextureFormat::RGBA16F, FBTextureLoadAction::Clear, FBTextureStoreAction::Store}, // SceneColor
	};
}*/

void LightingPass::Setup(RenderContext& ctx, FBAttachmentInfo& info)
{
	info.Width = m_Width;
	info.Height = m_Height;

	// Lighting Pass 是全屏绘制（Full-screen Quad），通常不需要深度测试
	info.Depth = {};

	info.DSS.depthWrite = false;
	info.DSS.depthTest = false;
	info.DSS.compareFunc = ECompareFunc::Less;

	// 输入：此时 ctx.GBuffer_Normal 等纹理已由前面 Pass 生成
	// 输出：如果前面 Skybox 已经画了，这里 LoadAction 应该是 Load，否则会覆盖天空
	info.Attachments = {
		{ &ctx.LightMap_SceneColor, CreateHDRBuffer(m_Width, m_Height), FBTextureLoadAction::Load, FBTextureStoreAction::Store }
	};
}

void LightingPass::Execute(Ref<Scene> scene)
{
	RenderContext& ctx = scene->GetRenderContext();

	int32 index = 0;
	for (auto lightActor : scene->GetActors())
	{
		for (auto& Comp : lightActor->GetComponents())
		{
			if (auto Light = std::dynamic_pointer_cast<DirectionLightComponent>(Comp))
			{
				LightInfo& info = ctx.LightDataUB->Data.lights[index++];
				info.position = Light->GetLocation();
				info.color = Light->GetColor();
				info.type = 0;
				info.intensity = Light->GetIntensity() * 10;
				info.direction = Light->GetDirection();
			}
		}
	}
	ctx.LightDataUB->Data.NumLights = index;
	ctx.LightDataUB->Update();

	uint32 freeSlot = m_Shader->GetFreeSlotIndex();

	m_Shader->Bind();
	ctx.GBuffer_Position->Bind(freeSlot);
	m_Shader->SetUniform1i("gPosition", freeSlot++);

	ctx.GBuffer_Normal->Bind(freeSlot);
	m_Shader->SetUniform1i("gNormal", freeSlot++);

	ctx.GBuffer_Albedo->Bind(freeSlot);
	m_Shader->SetUniform1i("gAlbedo", freeSlot++);

	ctx.GBuffer_Material->Bind(freeSlot);
	m_Shader->SetUniform1i("gMaterial", freeSlot++);

	ctx.ShadowMap_Depth->Bind(freeSlot);
	m_Shader->SetUniform1i("gShadowMap", freeSlot++);

	std::vector<uint32> indices = {0, 1, 2, 2, 1, 3};
	std::vector<float> vertices = {-1.0f,  1.0f, -1.0f, -1.0f, 1.0f,  1.0f, 1.0f, -1.0f,};
	Ref<MeshSection> section = MeshBuilder::BuildSection(vertices, indices, BufferLayout{BufferElement{ShaderDataType::Float2, "aPosition"}});
	section->Draw();
}