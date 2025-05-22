#pragma once
#include "BlockBase.h"

class Panel : public BlockBase {
public:
    Panel(float s, float o = 0.03f) : BlockBase(s, o) { hasAlpha = true; }
    void Init() {
        BlockBase::Init("textures/glass.png");
    }
protected:
    const char* VertexShaderSrc() override {
        return "#version 330 core\n"
            "layout(location=0) in vec3 p;"
            "layout(location=1) in vec2 uv;"
            "layout(location=2) in vec3 n;"
            "uniform mat4 model;"
            "uniform mat4 view;"
            "uniform mat4 projection;"
            "out vec2 TexCoord;"
            "void main(){"
            "gl_Position=projection*view*model*vec4(p,1.0);"
            "TexCoord=uv;"
            "}";
    }
    const char* FragmentShaderSrc() override {
        return "#version 330 core\n"
            "in vec2 TexCoord;"
            "out vec4 FragColor;"
            "uniform sampler2D ourTexture;"
            "void main(){"
            "vec4 c=texture(ourTexture,TexCoord);"
            "if(c.a<0.1) discard;"
            "FragColor=c;"
            "}";
    }
    void SetupBuffers() override {
        float half = size;
        float thickness = size * 0.05f;
        float bottom = -size;
        float top = size;
        float v[] = {
            -half, bottom, -thickness,   0,0,   0,0,-1,
             half, bottom, -thickness,   1,0,   0,0,-1,
             half,    top, -thickness,   1,1,   0,0,-1,
            -half,    top, -thickness,   0,1,   0,0,-1,
            -half, bottom,  thickness,   0,0,   0,0,1,
             half, bottom,  thickness,   1,0,   0,0,1,
             half,    top,  thickness,   1,1,   0,0,1,
            -half,    top,  thickness,   0,1,   0,0,1,
            -half,    top, -thickness,   0,0,   0,1,0,
             half,    top, -thickness,   1,0,   0,1,0,
             half,    top,  thickness,   1,1,   0,1,0,
            -half,    top,  thickness,   0,1,   0,1,0,
            -half, bottom, -thickness,   0,0,   0,-1,0,
             half, bottom, -thickness,   1,0,   0,-1,0,
             half, bottom,  thickness,   1,1,   0,-1,0,
            -half, bottom,  thickness,   0,1,   0,-1,0,
            -half, bottom, -thickness,   0,0,   -1,0,0,
            -half,    top, -thickness,   1,0,   -1,0,0,
            -half,    top,  thickness,   1,1,   -1,0,0,
            -half, bottom,  thickness,   0,1,   -1,0,0,
             half, bottom, -thickness,   0,0,   1,0,0,
             half,    top, -thickness,   1,0,   1,0,0,
             half,    top,  thickness,   1,1,   1,0,0,
             half, bottom,  thickness,   0,1,   1,0,0
        };
        unsigned int i[] = {
            0,1,2,2,3,0,
            4,5,6,6,7,4,
            8,9,10,10,11,8,
            12,13,14,14,15,12,
            16,17,18,18,19,16,
            20,21,22,22,23,20
        };
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(i), i, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    }
};
