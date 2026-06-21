#pragma once
#include "Engine.h"
#include "ImGui/imgui.h"
#include "Model/Texture.h"

inline ImTextureRef ToImTexture(const Ref<Texture>& tex)
{
	auto id = gEngine->GetDriver().GetNativeTextureId(tex->GetHandle());
	return ImTextureRef((ImTextureID)(uint64_t)id);
}
