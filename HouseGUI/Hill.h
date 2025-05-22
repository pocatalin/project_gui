// Hill.h
#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

class Hill {
public:
    Hill(float baseSize, float height, int segments, float exponent = 3.0f, float squareSize = 1.0f);
    ~Hill();
    void Init();
    void Draw(const glm::mat4& view,
        const glm::mat4& proj,
        const glm::mat4& model,
        const glm::mat4& lightSpaceMatrix,
        const glm::vec3& lightDir,
        const glm::vec3& lightColor,
        const glm::vec3& viewPos,
        GLuint shadowMap);
private:
    float baseSize;
    float height;
    float exponent;
    float squareSize;
    int segments;
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    GLuint shader;
    GLuint textureID;
    GLuint indexCount;
    void generateMesh();
    GLuint compileShader(const char* src, GLenum type);
    GLuint createProgram(const char* vs, const char* fs);
};
