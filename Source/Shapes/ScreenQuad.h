#pragma once
#include "common/Core.h"

class MeshSection;

class ScreenQuad
{
public:
	ScreenQuad();
	void Draw() const;

private:
	Ref<MeshSection> m_MeshSection;
};
