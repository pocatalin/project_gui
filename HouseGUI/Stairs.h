#pragma once
#include "BlockBase.h"
class Stairs : public BlockBase {
public:
    Stairs(float s, float o = 0.03f) :BlockBase(s, o) { hasAlpha = false; }
    void Init() { BlockBase::Init("textures/planks_oak.png", "textures/planks_oak.png", "textures/planks_oak.png"); }
};
