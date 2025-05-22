// SmoothPyramid.h
#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
class SmoothPyramid {
public:
    SmoothPyramid(float height, float baseRadius, int radialSegments, int heightSegments);
    ~SmoothPyramid();
    void Init();
    void Draw(const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model, const glm::vec3& lightDir, const glm::vec3& lightColor, const glm::vec3& viewPos);
private:
    float height, baseRadius;
    int radialSegments, heightSegments;
    GLuint VAO, VBO, EBO, shader;
    GLuint indexCount;
    void generateMesh();
    GLuint compileShader(const char* source, GLenum type);
    GLuint createProgram(const char* vsource, const char* fsource);
};
