#pragma once
#include "BlockBase.h"

class OakLog : public BlockBase {
public:
    OakLog(float s, float o = 0.03f) : BlockBase(s, o) { hasAlpha = false; }
    void Init() { BlockBase::Init("textures/log_oak_top.png", "textures/log_oak.png", "textures/log_oak_top.png"); }
};
