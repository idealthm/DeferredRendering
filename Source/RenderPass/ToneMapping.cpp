#include "ToneMapping.h"

#include "Renderer.h"
#include "Scene.h"
#include "FrameBuffer/FrameBuffer.h"
#include "Model/Texture.h"
#include "Shader/Program.h"

using namespace TextureFactory;

ToneMapping::ToneMapping()
{
}

void ToneMapping::Setup(FBAttachmentInfo& info, uint32_t step, RenderContext& ctx)
{
	info.Depth = {};
 
	info.DSS.depthTest = false;
	info.DSS.depthWrite = false;

	CreateResource(ctx.Final_SceneColor, CreateFinalColor(info.Width, info.Height));

	info.Attachments = {
		// {ctx.Final_SceneColor->GetRendererID(), FBTextureLoadAction::Clear, FBTextureStoreAction::Store},
	};
}

void ToneMapping::Execute(Ref<Scene> scene, uint32_t step, RenderContext& ctx)
{
}
