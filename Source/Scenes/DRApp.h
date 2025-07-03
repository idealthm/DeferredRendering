#pragma once
#include "Common/Core.h"

class DRApp
{
protected:
    DRApp();

    void CalcFPS();

    void RenderFPS();

    float GetRunningTime();

private:
    int64 m_FrameTime;
    int64 m_StartTime;
    int32 m_FrameCount;
    int32 m_FPS;
};
