#pragma once
#include "Common/Keys.h"
#include "Common/MouseCode.h"

class Input
{
public:
	static bool IsKeyPressed(KeyCode key);

	static bool IsMouseButtonPressed(MouseCode button);
	static float GetMouseX();
	static float GetMouseY();
};
