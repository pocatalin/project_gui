#pragma once
#include "BlockBase.h"

class Leaves : public BlockBase {
public:
    Leaves(float s, float o = 0.03f) : BlockBase(s, o) { hasAlpha = false; }
    void Init() { BlockBase::Init("textures/azalea_leaves.png", "textures/azalea_leaves.png", "textures/azalea_leaves.png"); }
};
