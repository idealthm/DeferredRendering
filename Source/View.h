#pragma once
#include "Camera/Camera.h"
#include "Common/Core.h"

class View
{
public:
	void SetScene();
	void SetCamera(Ref<Camera> camera);
	void SetRenderTarget();
	void SetViewport();
	
	void GetScene();
	void GetViewport();
	void GetCamera();
};
