#pragma once
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb/stb_image.h"

class BlockBase {
public:
    unsigned int VAO, VBO, EBO;
    unsigned int topID, sideID, bottomID;
    unsigned int shaderProgram;
    float size;
    bool hasAlpha;
    float outlineSize;
    BlockBase(float s, float o = 0.03f);
    virtual ~BlockBase();
    virtual void Init(const char* t, const char* si, const char* b);
    virtual void Init(const std::string& tex);
    virtual void Draw(const glm::mat4& view, const glm::mat4& proj, const glm::mat4& model,
        const glm::mat4& lightSpaceMatrix, const glm::vec3& lightDir,
        const glm::vec3& lightColor, const glm::vec3& viewPos, GLuint shadowMap);
protected:
    virtual const char* VertexShaderSrc();
    virtual const char* FragmentShaderSrc();
    virtual void SetupShaders();
    virtual void SetupBuffers();
    void LoadTexture(const char* path, unsigned int& texID);
    void Cleanup();
};
