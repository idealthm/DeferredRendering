#include "ToneMapping.h"

#include "Renderer.h"
#include "Scene.h"
#include "FrameBuffer/FrameBuffer.h"
#include "glad/glad.h"
#include "Model/MeshSection.h"
#include "Shapes/MeshBuilder.h"

ToneMapping::ToneMapping(uint32 width, uint32 height)
	: RenderPass(width, height)
{
	m_Shader = CreateRef<Shader>("Shaders/Passes/ToneMapping", 0);
}

void ToneMapping::Setup(RenderContext& ctx, FBAttachmentInfo& info)
{
	info.Width = m_Width;
	info.Height = m_Height;
// 
	// info.Depth = {};
// 
	// info.DSS.depthTest = false;
	// info.DSS.depthWrite = false;
// 
	// info.Attachments = {
	// 	{&ctx.Final_SceneColor, CreateFinalColor(m_Width, m_Height), FBTextureLoadAction::Clear, FBTextureStoreAction::Store},
	// };
}

void ToneMapping::Execute(Ref<Scene> scene)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_Shader->Bind();

	RenderContext& ctx = scene->GetRenderContext();
	int32 freeIndex = m_Shader->GetFreeSlotIndex();

	ctx.LightMap_SceneColor->Bind(freeIndex);
	m_Shader->SetUniform1i("uHdrSceneColor", freeIndex++);

	std::vector<uint32> indices = {0, 1, 2, 1, 2, 3};
	std::vector<float> vertices = {-1.0f,  1.0f, -1.0f, -1.0f, 1.0f,  1.0f, 1.0f, -1.0f,};
	Ref<MeshSection> section = MeshBuilder::BuildSection(vertices, indices, BufferLayout{BufferElement{ShaderDataType::Float2, "aPosition"}});
	section->Draw();
}
