#include "LightingPass.h"

#include "GBufferPass.h"
#include "Scene.h"
#include "Common/StringFormat.h"
#include "FrameBuffer/FrameBuffer.h"
#include "glad/glad.h"
#include "Lights/Light.h"
#include "Shader/Shader.h"
#include "Shapes/MeshBuilder.h"

LightingPass::LightingPass(uint32 width, uint32 height)
	: RenderPass(width, height)
{
	m_LightingShader = CreateScope<Shader>("Shaders/DirectionLight.glsl");
}

void LightingPass::OnWindowSizeChanged(int32 width, int32 height)
{
	
}

void LightingPass::PrePass(RenderContext& context)
{
	RenderPass::PrePass(context);
}

void LightingPass::OnPass(RenderContext& context)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_LightingShader->Bind();

	int32 pointlightIndex = 0, spotlightIndex = 0;
	for (const auto& Actor : context.scene->GetActors())
	{
		for (auto& Comp : Actor->GetComponents())
		{
			if (auto Light = std::dynamic_pointer_cast<DirectionLightComponent>(Comp))
			{
				m_LightingShader->SetUniform3f("dirLight.direction", Light->GetDirection());
				m_LightingShader->SetUniform3f("dirLight.diffuse", Light->m_Diffuse);
				m_LightingShader->SetUniform3f("dirLight.ambient", Light->m_Ambient);
				m_LightingShader->SetUniform3f("dirLight.specular", Light->m_Specular);
			}
			if (auto Light = std::dynamic_pointer_cast<PointLightComponent>(Comp))
			{
				m_LightingShader->SetUniform3f(String::Printf("pointLights[%d].position", pointlightIndex), Light->GetLocation());
				m_LightingShader->SetUniform3f(String::Printf("pointLights[%d].diffuse", pointlightIndex), Light->m_Diffuse);
				m_LightingShader->SetUniform3f(String::Printf("pointLights[%d].ambient", pointlightIndex), Light->m_Ambient);
				m_LightingShader->SetUniform3f(String::Printf("pointLights[%d].specular", pointlightIndex), Light->m_Specular);
				m_LightingShader->SetUniform1f(String::Printf("pointLights[%d].constant", pointlightIndex), Light->m_Constant);
				m_LightingShader->SetUniform1f(String::Printf("pointLights[%d].linear", pointlightIndex), Light->m_Linear);
				m_LightingShader->SetUniform1f(String::Printf("pointLights[%d].quadratic", pointlightIndex), Light->m_Quadratic);
				pointlightIndex ++;
			}
			if (auto Light = std::dynamic_pointer_cast<SpotLightComponent>(Comp))
			{
				m_LightingShader->SetUniform3f(String::Printf("spotLights[%d].position", spotlightIndex), Light->GetLocation());
				m_LightingShader->SetUniform3f(String::Printf("spotLights[%d].direction", spotlightIndex), Light->GetDirection());
				m_LightingShader->SetUniform3f(String::Printf("spotLights[%d].diffuse", spotlightIndex), Light->m_Diffuse);
				m_LightingShader->SetUniform3f(String::Printf("spotLights[%d].ambient", spotlightIndex), Light->m_Ambient);
				m_LightingShader->SetUniform3f(String::Printf("spotLights[%d].specular", spotlightIndex), Light->m_Specular);
				m_LightingShader->SetUniform1f(String::Printf("spotLights[%d].constant", spotlightIndex), Light->m_Constant);
				m_LightingShader->SetUniform1f(String::Printf("spotLights[%d].linear", spotlightIndex), Light->m_Linear);
				m_LightingShader->SetUniform1f(String::Printf("spotLights[%d].quadratic", spotlightIndex), Light->m_Quadratic);
				m_LightingShader->SetUniform1f(String::Printf("spotLights[%d].cutoff", spotlightIndex), Light->m_Cutoff);
				m_LightingShader->SetUniform1f(String::Printf("spotLights[%d].outerCutoff", spotlightIndex), Light->m_OuterCutoff);
				spotlightIndex ++;
			}
		}
	}

	m_LightingShader->SetUniform1i("uDebugMode", context.renderMode);

	m_LightingShader->SetUniform3f("uCamPos", context.scene->GetCameraPosition());
	m_LightingShader->SetUniform1i("uPointLightCount", pointlightIndex);
	m_LightingShader->SetUniform1i("uSpotLightCount", spotlightIndex);

	Ref<GBufferPass> GBufferPass = Renderer::Get().GetGBufferPass();

	auto BindForReading = [this, GBufferPass](const std::string& name, uint32 slot)
	{
		m_LightingShader->SetUniform1i(name, slot);
		GLCall(glBindTextureUnit(slot, GBufferPass->GetColorAttachmentRendererID(name)))
	};

	BindForReading(std::string("gPosition"), context.usedTextureSlot++);
	BindForReading(std::string("gNormal"), context.usedTextureSlot++);
	BindForReading(std::string("gAlbedo"), context.usedTextureSlot++);

	StaticMeshActor quadActor;
	quadActor.SetStaticMesh(MeshBuilder::BuildQuad());
	quadActor.GetRootComponent()->Draw(*m_LightingShader);
}

void LightingPass::PostPass(RenderContext& context)
{
	RenderPass::PostPass(context);
}
