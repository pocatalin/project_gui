// SmoothPyramid.cpp
#include "SmoothPyramid.h"
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
static glm::vec2 catmullRom(const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, float t) {
    float t2 = t * t, t3 = t2 * t;
    return 0.5f * ((2.0f * p1) + (-p0 + p2) * t + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 + (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}
SmoothPyramid::SmoothPyramid(float h, float r, int rs, int hs)
    : height(h), baseRadius(r), radialSegments(rs), heightSegments(hs),
    VAO(0), VBO(0), EBO(0), shader(0), indexCount(0) {
}
SmoothPyramid::~SmoothPyramid() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
    if (shader) glDeleteProgram(shader);
}
void SmoothPyramid::Init() {
    generateMesh();
    const char* vs = R"(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec3 FragPos;
out vec3 Normal;
void main(){
    FragPos=vec3(model*vec4(aPos,1.0));
    Normal=mat3(transpose(inverse(model)))*aNormal;
    gl_Position=projection*view*vec4(FragPos,1.0);
}
)";
    const char* fs = R"(
#version 330 core
in vec3 FragPos;
in vec3 Normal;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos;
out vec4 FragColor;
void main(){
    vec3 norm=normalize(Normal);
    vec3 ld=normalize(-lightDir);
    float diff=max(dot(norm,ld),0.0);
    vec3 diffuse=diff*lightColor;
    vec3 viewD=normalize(viewPos-FragPos);
    vec3 refl=reflect(-ld,norm);
    float spec=pow(max(dot(viewD,refl),0.0),32.0);
    vec3 specular=spec*lightColor;
    vec3 ambient=0.1*lightColor;
    vec3 color=vec3(0.3,0.8,0.2)*(ambient+diffuse+specular);
    FragColor=vec4(color,1.0);
}
)";
    shader = createProgram(vs, fs);
}
void SmoothPyramid::generateMesh() {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;
    glm::vec2 p0 = { baseRadius,0.0f }, p1 = { 0.0f,height }, c0 = p1, c1 = p1, c2 = p0, c3 = p0;
    for (int y = 0; y <= heightSegments; ++y) {
        float t = (float)y / heightSegments;
        glm::vec2 sp = catmullRom(c0, c1, c2, c3, t);
        float r = sp.x, yy = sp.y;
        for (int i = 0; i <= radialSegments; ++i) {
            float a = (float)i / radialSegments * 2.0f * 3.14159265359f;
            float x = r * cos(a), z = r * sin(a);
            vertices.emplace_back(x, yy, z);
            normals.push_back(glm::normalize(glm::vec3(x, yy - height, z)));
        }
    }
    for (int y = 0; y < heightSegments; ++y) {
        for (int i = 0; i < radialSegments; ++i) {
            int a = y * (radialSegments + 1) + i, b = a + radialSegments + 1;
            indices.push_back(a); indices.push_back(b); indices.push_back(a + 1);
            indices.push_back(a + 1); indices.push_back(b); indices.push_back(b + 1);
        }
    }
    indexCount = indices.size();
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3) + normals.size() * sizeof(glm::vec3), nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(glm::vec3), vertices.data());
    glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), normals.size() * sizeof(glm::vec3), normals.data());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(vertices.size() * sizeof(glm::vec3)));
    glBindVertexArray(0);
}
GLuint SmoothPyramid::compileShader(const char* src, GLenum t) {
    GLuint s = glCreateShader(t);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    return s;
}
GLuint SmoothPyramid::createProgram(const char* vs, const char* fs) {
    GLuint v = compileShader(vs, GL_VERTEX_SHADER), f = compileShader(fs, GL_FRAGMENT_SHADER);
    GLuint p = glCreateProgram();
    glAttachShader(p, v); glAttachShader(p, f);
    glLinkProgram(p);
    glDeleteShader(v); glDeleteShader(f);
    return p;
}
void SmoothPyramid::Draw(const glm::mat4& view, const glm::mat4& proj, const glm::mat4& model, const glm::vec3& lightDir, const glm::vec3& lightColor, const glm::vec3& viewPos) {
    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniform3fv(glGetUniformLocation(shader, "lightDir"), 1, glm::value_ptr(lightDir));
    glUniform3fv(glGetUniformLocation(shader, "lightColor"), 1, glm::value_ptr(lightColor));
    glUniform3fv(glGetUniformLocation(shader, "viewPos"), 1, glm::value_ptr(viewPos));
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
