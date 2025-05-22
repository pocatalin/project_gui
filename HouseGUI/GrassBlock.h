#pragma once
#include "BlockBase.h"
#include <iostream>

class GrassBlock : public BlockBase {
public:
    GrassBlock(float s, float o = 0.03f) : BlockBase(s, o) { hasAlpha = false; }
    void Init() {
        LoadTexture("textures/grass_carried.png", topID);
        stbi_set_flip_vertically_on_load(true);
        LoadTexture("textures/grass_side_carried.png", sideID);
        LoadTexture("textures/dirt.png", bottomID);
        SetupShaders();
        SetupBuffers();
    }
};
