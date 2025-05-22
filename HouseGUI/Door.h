#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb/stb_image.h"

class Door {
public:
    Door(float s) : size(s), bottomTex(0), topTex(0), shaderProgram(0), VAO1(0), VBO1(0), EBO1(0), VAO2(0), VBO2(0), EBO2(0) {}
    void Init() {
        LoadTexture("textures/door_wood_lower.png", bottomTex);
        LoadTexture("textures/door_wood_upper.png", topTex);
        SetupShaders();
        SetupBuffers();
    }
    void Draw(const glm::mat4& view, const glm::mat4& proj, const glm::mat4& model,
        const glm::mat4& lightSpaceMatrix, const glm::vec3& lightDir,
        const glm::vec3& lightColor, const glm::vec3& viewPos, GLuint shadowMap) {
        glUseProgram(shaderProgram);
        GLint mLoc = glGetUniformLocation(shaderProgram, "model");
        GLint vLoc = glGetUniformLocation(shaderProgram, "view");
        GLint pLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(proj));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bottomTex);
        glBindVertexArray(VAO1);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, topTex);
        glBindVertexArray(VAO2);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
private:
    float size;
    unsigned int bottomTex, topTex;
    unsigned int shaderProgram;
    unsigned int VAO1, VBO1, EBO1;
    unsigned int VAO2, VBO2, EBO2;
    void LoadTexture(const char* path, unsigned int& texID) {
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        int w, h, n;
        unsigned char* data = stbi_load(path, &w, &h, &n, 4);
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        stbi_image_free(data);
    }
    void SetupShaders() {
        const char* vsSrc =
            "#version 330 core\n"
            "layout(location=0) in vec3 aPos;"
            "layout(location=1) in vec2 aTex;"
            "uniform mat4 model;"
            "uniform mat4 view;"
            "uniform mat4 projection;"
            "out vec2 TexCoord;"
            "void main(){"
            "gl_Position=projection*view*model*vec4(aPos,1.0);"
            "TexCoord=aTex;"
            "}";
        const char* fsSrc =
            "#version 330 core\n"
            "in vec2 TexCoord;"
            "out vec4 FragColor;"
            "uniform sampler2D ourTexture;"
            "void main(){"
            "vec4 c=texture(ourTexture,TexCoord);"
            "if(c.a<0.1) discard;"
            "FragColor=c;"
            "}";
        unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &vsSrc, nullptr);
        glCompileShader(vs);
        unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &fsSrc, nullptr);
        glCompileShader(fs);
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vs);
        glAttachShader(shaderProgram, fs);
        glLinkProgram(shaderProgram);
        glDeleteShader(vs);
        glDeleteShader(fs);
    }
    void SetupBuffers() {
        float thick = 0.015f;
        float halfW = 0.2f;
        float bottomY = 0.0f;
        float midY = 0.45f;
        float topY = 0.9f;
        float v1[] = {
            -halfW, bottomY, -thick, 0,0,
             halfW, bottomY, -thick, 1,0,
             halfW,     midY, -thick, 1,1,
            -halfW,     midY, -thick, 0,1,
            -halfW, bottomY,  thick, 0,0,
             halfW, bottomY,  thick, 1,0,
             halfW,     midY,  thick, 1,1,
            -halfW,     midY,  thick, 0,1,
            -halfW,     midY, -thick, 0,0,
             halfW,     midY, -thick, 1,0,
             halfW,     midY,  thick, 1,1,
            -halfW,     midY,  thick, 0,1,
            -halfW, bottomY, -thick, 0,0,
             halfW, bottomY, -thick, 1,0,
             halfW, bottomY,  thick, 1,1,
            -halfW, bottomY,  thick, 0,1,
            -halfW, bottomY, -thick, 0,0,
            -halfW,     midY, -thick, 1,0,
            -halfW,     midY,  thick, 1,1,
            -halfW, bottomY,  thick, 0,1,
             halfW, bottomY, -thick, 0,0,
             halfW,     midY, -thick, 1,0,
             halfW,     midY,  thick, 1,1,
             halfW, bottomY,  thick, 0,1
        };
        unsigned int ind[] = {
            0,1,2,2,3,0,
            4,5,6,6,7,4,
            8,9,10,10,11,8,
            12,13,14,14,15,12,
            16,17,18,18,19,16,
            20,21,22,22,23,20
        };
        glGenVertexArrays(1, &VAO1);
        glGenBuffers(1, &VBO1);
        glGenBuffers(1, &EBO1);
        glBindVertexArray(VAO1);
        glBindBuffer(GL_ARRAY_BUFFER, VBO1);
        glBufferData(GL_ARRAY_BUFFER, sizeof(v1), v1, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO1);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ind), ind, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
        float v2[] = {
            -halfW,     midY, -thick, 0,0,
             halfW,     midY, -thick, 1,0,
             halfW,     topY, -thick, 1,1,
            -halfW,     topY, -thick, 0,1,
            -halfW,     midY,  thick, 0,0,
             halfW,     midY,  thick, 1,0,
             halfW,     topY,  thick, 1,1,
            -halfW,     topY,  thick, 0,1,
            -halfW,     topY, -thick, 0,0,
             halfW,     topY, -thick, 1,0,
             halfW,     topY,  thick, 1,1,
            -halfW,     topY,  thick, 0,1,
            -halfW,     midY, -thick, 0,0,
             halfW,     midY, -thick, 1,0,
             halfW,     midY,  thick, 1,1,
            -halfW,     midY,  thick, 0,1,
            -halfW,     midY, -thick, 0,0,
            -halfW,     topY, -thick, 1,0,
            -halfW,     topY,  thick, 1,1,
            -halfW,     midY,  thick, 0,1,
             halfW,     midY, -thick, 0,0,
             halfW,     topY, -thick, 1,0,
             halfW,     topY,  thick, 1,1,
             halfW,     midY,  thick, 0,1
        };
        glGenVertexArrays(1, &VAO2);
        glGenBuffers(1, &VBO2);
        glGenBuffers(1, &EBO2);
        glBindVertexArray(VAO2);
        glBindBuffer(GL_ARRAY_BUFFER, VBO2);
        glBufferData(GL_ARRAY_BUFFER, sizeof(v2), v2, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ind), ind, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }
};
