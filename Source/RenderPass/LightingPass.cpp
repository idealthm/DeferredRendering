#include "LightingPass.h"

#include "Scene.h"
#include "FrameBuffer/FrameBuffer.h"
#include "Lights/Light.h"
#include "Model/Texture.h"
#include "Shader/Shader.h"
#include "Shapes/MeshBuilder.h"
#include "glad/glad.h"
#include "Shader/ShaderLibrary.h"

LightingPass::LightingPass()
{
	m_Shader = ShaderLibrary::Get().GetShader("Shaders/DirectionLight", nullptr);
}

void LightingPass::Setup(FBAttachmentInfo& info)
{
	// Lighting Pass 是全屏绘制（Full-screen Quad），通常不需要深度测试
	info.Depth = {};

	info.DSS.depthWrite = false;
	info.DSS.depthTest = false;
	info.DSS.compareFunc = ECompareFunc::Less;

	// 输入：此时 ctx.GBuffer_Normal 等纹理已由前面 Pass 生成
	// 输出：如果前面 Skybox 已经画了，这里 LoadAction 应该是 Load，否则会覆盖天空
	info.Attachments = {
		{ &g_ctx.LightMap_SceneColor, CreateHDRBuffer(info.Width, info.Height), FBTextureLoadAction::Load, FBTextureStoreAction::Store }
	};
}

void LightingPass::Execute(Ref<Scene> scene)
{
	int32 index = 0;
	for (auto lightActor : scene->GetActors())
	{
		for (auto& Comp : lightActor->GetComponents())
		{
			if (auto Light = std::dynamic_pointer_cast<DirectionLightComponent>(Comp))
			{
				LightInfo& info = g_ctx.LightDataUB->Data.lights[index++];
				info.position = Light->GetLocation();
				info.color = Light->GetColor();
				info.type = 0;
				info.intensity = Light->GetIntensity() * 10;
				info.direction = Light->GetDirection();
			}
		}
	}
	g_ctx.LightDataUB->Data.NumLights = index;
	g_ctx.LightDataUB->Update();

	uint32 freeSlot = m_Shader->GetFreeSlotIndex();

	m_Shader->Bind();
	g_ctx.GBuffer_Position->Bind(freeSlot);
	m_Shader->SetUniform1i("gPosition", freeSlot++);

	g_ctx.GBuffer_Normal->Bind(freeSlot);
	m_Shader->SetUniform1i("gNormal", freeSlot++);

	g_ctx.GBuffer_Albedo->Bind(freeSlot);
	m_Shader->SetUniform1i("gAlbedo", freeSlot++);

	g_ctx.GBuffer_Material->Bind(freeSlot);
	m_Shader->SetUniform1i("gMaterial", freeSlot++);

	g_ctx.ShadowMap_Depth->Bind(freeSlot);
	m_Shader->SetUniform1i("gShadowMap", freeSlot++);

	std::vector<uint32> indices = {0, 1, 2, 2, 1, 3};
	std::vector<float> vertices = {-1.0f,  1.0f, -1.0f, -1.0f, 1.0f,  1.0f, 1.0f, -1.0f,};
	Ref<MeshSection> section = MeshBuilder::BuildSection(vertices, indices, BufferLayout{BufferElement{ShaderDataType::Float2, "aPosition"}});
	section->Draw();
}