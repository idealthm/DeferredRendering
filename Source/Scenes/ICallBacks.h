#pragma once

#include "Common/Keys.h"

class ICallBacks
{
public:
    virtual ~ICallBacks();

    virtual void KeyBoardCB(DR_KEY Key, DR_KEY_STATE key_state=KEY_STATE_PRESS) {}

    virtual void PassiveMouseCB(int x, int y) {}

    virtual void RenderSceneCB() {}

    virtual void MouseCB(DR_MOUSE Button, DR_KEY_STATE State, int x, int y) {}
};
