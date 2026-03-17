#include "ToneMapping.h"

#include "Renderer.h"
#include "Scene.h"
#include "FrameBuffer/FrameBuffer.h"
#include "glad/glad.h"
#include "Model/MeshSection.h"
#include "Shader/ShaderLibrary.h"
#include "Shapes/MeshBuilder.h"

ToneMapping::ToneMapping()
{
	m_Shader = ShaderLibrary::Get().GetShader("Shaders/Passes/ToneMapping", nullptr);
}

void ToneMapping::Setup(FBAttachmentInfo& info)
{
	info.Depth = {};
 
	info.DSS.depthTest = false;
	info.DSS.depthWrite = false;
 
	info.Attachments = {
		{&g_ctx.Final_SceneColor, CreateFinalColor(info.Width, info.Height), FBTextureLoadAction::Clear, FBTextureStoreAction::Store},
	};
}

void ToneMapping::Execute(Ref<Scene> scene)
{
	m_Shader->Bind();

	int32 freeIndex = m_Shader->GetFreeSlotIndex();

	g_ctx.LightMap_SceneColor->Bind(freeIndex);
	m_Shader->SetUniform1i("uHdrSceneColor", freeIndex++);

	std::vector<uint32> indices = {0, 1, 2, 1, 2, 3};
	std::vector<float> vertices = {-1.0f,  1.0f, -1.0f, -1.0f, 1.0f,  1.0f, 1.0f, -1.0f,};
	Ref<MeshSection> section = MeshBuilder::BuildSection(vertices, indices, BufferLayout{BufferElement{ShaderDataType::Float2, "aPosition"}});
	section->Draw();
}
