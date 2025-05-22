// Robot.cpp
#include "Robot.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>

extern float getTerrainHeight(float x, float z);

static const char* vertSrc = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
out vec2 vUV;
void main() {
    vUV = aUV;
    gl_Position = uProj * uView * uModel * vec4(aPos, 1.0);
}
)";

static const char* fragSrc = R"(
#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D uTex;
uniform vec3 uColor;
uniform int useTex;
void main() {
    if (useTex == 1)
        FragColor = texture(uTex, vUV);
    else
        FragColor = vec4(uColor, 1.0);
}
)";

Robot::Robot(int, int, float scale)
    : uniformScale(scale), Velocity(0.0f), Position(0.0f), Yaw(0.0f), walkCycle(0.0f)
{
    init();
    loadHeadTextures();
    loadHeadOverlayTexture();
}

Robot::~Robot() {
    glDeleteProgram(shader);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteTextures(6, headTextures);
    glDeleteTextures(1, &headOverlayTexture);
}

GLuint Robot::compile(const char* src, GLenum type) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    return s;
}

void Robot::init() {
    GLuint vs = compile(vertSrc, GL_VERTEX_SHADER);
    GLuint fs = compile(fragSrc, GL_FRAGMENT_SHADER);
    shader = glCreateProgram();
    glAttachShader(shader, vs);
    glAttachShader(shader, fs);
    glLinkProgram(shader);
    glDeleteShader(vs);
    glDeleteShader(fs);

    locModel = glGetUniformLocation(shader, "uModel");
    locView = glGetUniformLocation(shader, "uView");
    locProj = glGetUniformLocation(shader, "uProj");
    useTex = glGetUniformLocation(shader, "useTex");
    locColor = glGetUniformLocation(shader, "uColor");

    glUseProgram(shader);
    glUniform1i(useTex, 1);

    float verts[] = {
        -0.5f,-0.5f, 0.5f, 0,0,  0.5f,-0.5f, 0.5f, 1,0,  0.5f, 0.5f, 0.5f, 1,1,  -0.5f, 0.5f, 0.5f, 0,1,
        -0.5f,-0.5f,-0.5f, 1,0, -0.5f, 0.5f,-0.5f, 1,1,  0.5f, 0.5f,-0.5f, 0,1,  0.5f,-0.5f,-0.5f, 0,0,
        -0.5f, 0.5f, 0.5f, 1,0, -0.5f, 0.5f,-0.5f, 1,1, -0.5f,-0.5f,-0.5f, 0,1, -0.5f,-0.5f, 0.5f, 0,0,
         0.5f, 0.5f, 0.5f, 0,0,  0.5f,-0.5f, 0.5f, 0,1,  0.5f,-0.5f,-0.5f, 1,1,  0.5f, 0.5f,-0.5f, 1,0,
        -0.5f, 0.5f,-0.5f, 0,1, -0.5f, 0.5f, 0.5f, 0,0,  0.5f, 0.5f, 0.5f, 1,0,  0.5f, 0.5f,-0.5f, 1,1,
        -0.5f,-0.5f,-0.5f, 1,1,  0.5f,-0.5f,-0.5f, 0,1,  0.5f,-0.5f, 0.5f, 0,0,  -0.5f,-0.5f, 0.5f, 1,0
    };
    unsigned int idx[] = {
         0,1,2,2,3,0, 4,5,6,6,7,4, 8,9,10,10,11,8,
        12,13,14,14,15,12, 16,17,18,18,19,16, 20,21,22,22,23,20
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Robot::loadHeadTextures() {
    const char* paths[6] = {
        "textures/skin/head_back.png",
        "textures/skin/head_front.png",
        "textures/skin/head_left.png",
        "textures/skin/head_right.png",
        "textures/skin/head_top.png",
        "textures/skin/head_bottom.png"
    };
    glGenTextures(6, headTextures);
    for (int i = 0; i < 6; ++i) {
        int w, h, c;
        unsigned char* data = stbi_load(paths[i], &w, &h, &c, 4);
        glBindTexture(GL_TEXTURE_2D, headTextures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
}

void Robot::loadHeadOverlayTexture() {
    int w, h, c;
    unsigned char* data = stbi_load("textures/skin/head_eyebrows.png", &w, &h, &c, 4);
    glGenTextures(1, &headOverlayTexture);
    glBindTexture(GL_TEXTURE_2D, headOverlayTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void Robot::drawHead(const glm::mat4& model, const glm::vec3& offset, const glm::vec3& scale, HeadFace face) {
    glUniform1i(useTex, 1);
    glBindTexture(GL_TEXTURE_2D, headTextures[int(face)]);
    glm::mat4 m = glm::translate(model, offset) * glm::scale(glm::mat4(1.0f), scale);
    glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(m));
    glBindVertexArray(VAO);
    int fi = int(face);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(fi * 6 * sizeof(unsigned int)));
    if (face == HeadFace::Front) {
        glBindTexture(GL_TEXTURE_2D, headOverlayTexture);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(fi * 6 * sizeof(unsigned int)));
    }
    glUniform1i(useTex, 0);
}

void Robot::drawCubeColor(const glm::mat4& model, const glm::vec3& offset, const glm::vec3& scale, const glm::vec3& color) {
    glUniform1i(useTex, 0);
    glm::mat4 m = glm::translate(model, offset) * glm::scale(glm::mat4(1.0f), scale);
    glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(m));
    glUniform3fv(locColor, 1, glm::value_ptr(color));
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
}

void Robot::drawLimb(const glm::mat4& parent, const glm::vec3& offset, float yaw, float pitch, float elbow,
    const glm::vec3& upperSize, const glm::vec3& lowerSize,
    const glm::vec3& upperColor, const glm::vec3& lowerColor) {
    const float gap = 0.01f;
    glm::mat4 m = glm::translate(parent, offset);
    m = glm::rotate(m, yaw, glm::vec3(0, 1, 0));
    m = glm::rotate(m, pitch, glm::vec3(1, 0, 0));
    drawCubeColor(m, glm::vec3(0, -upperSize.y / 2 - gap / 2, 0), upperSize, upperColor);
    glm::mat4 e = glm::translate(m, glm::vec3(0, -upperSize.y - gap, 0));
    e = glm::rotate(e, elbow, glm::vec3(1, 0, 0));
    drawCubeColor(e, glm::vec3(0, -lowerSize.y / 2 + gap / 2, 0), lowerSize, lowerColor);
}

void Robot::Update(float dt, GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  Yaw += TurnSpeed * dt;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) Yaw -= TurnSpeed * dt;
    glm::vec3 dir(0.0f);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    dir += glm::vec3(sinf(glm::radians(Yaw)), 0, cosf(glm::radians(Yaw)));
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  dir -= glm::vec3(sinf(glm::radians(Yaw)), 0, cosf(glm::radians(Yaw)));
    if (glm::length(dir) > 0.0f) {
        Velocity.x = dir.x * MoveSpeed;
        Velocity.z = dir.z * MoveSpeed;
    }
    else {
        Velocity.x = 0;
        Velocity.z = 0;
    }
    Velocity.y += Gravity * dt;
    Position += Velocity * dt;
    float ground = getTerrainHeight(Position.x, Position.z) + BaseOffset;
    if (Position.y <= ground) {
        Position.y = ground;
        Velocity.y = 0;
    }
    if (glm::length(glm::vec2(Velocity.x, Velocity.z)) > 0.1f) {
        walkCycle += dt * 6.0f;
    }
}

void Robot::Draw(const glm::mat4& view, const glm::mat4& proj) {
    glUseProgram(shader);
    glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(locProj, 1, GL_FALSE, glm::value_ptr(proj));
    glm::mat4 model = glm::translate(glm::mat4(1.0f), Position);
    model = glm::rotate(model, glm::radians(180.0f) + Yaw, glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(uniformScale));
    float swing = sinf(walkCycle) * 0.5f;
    float elbowSwing = -swing;
    drawHead(model, { 0,5,0 }, { 4,4,4 }, HeadFace::Front);
    drawHead(model, { 0,5,0 }, { 4,4,4 }, HeadFace::Back);
    drawHead(model, { 0,5,0 }, { 4,4,4 }, HeadFace::Left);
    drawHead(model, { 0,5,0 }, { 4,4,4 }, HeadFace::Right);
    drawHead(model, { 0,5,0 }, { 4,4,4 }, HeadFace::Top);
    drawHead(model, { 0,5,0 }, { 4,4,4 }, HeadFace::Bottom);
    drawCubeColor(model, { 0,0,0 }, { 4,6,2 }, { 14 / 255.0f,174 / 255.0f,174 / 255.0f });
    drawLimb(model, { -3,3,0 }, 0, swing, elbowSwing, { 2,4,2 }, { 2,3,2 }, { 14 / 255.0f,174 / 255.0f,174 / 255.0f }, { 169 / 255.0f,125 / 255.0f,100 / 255.0f });
    drawLimb(model, { 3,3,0 }, 0, -swing, -elbowSwing, { 2,4,2 }, { 2,3,2 }, { 14 / 255.0f,174 / 255.0f,174 / 255.0f }, { 169 / 255.0f,125 / 255.0f,100 / 255.0f });
    drawLimb(model, { -1,-3,0 }, 0, -swing, 0, { 2,3,2 }, { 2,4,2 }, { 73 / 255.0f,70 / 255.0f,151 / 255.0f }, { 73 / 255.0f,70 / 255.0f,151 / 255.0f });
    drawLimb(model, { 1,-3,0 }, 0, swing, 0, { 2,3,2 }, { 2,4,2 }, { 73 / 255.0f,70 / 255.0f,151 / 255.0f }, { 73 / 255.0f,70 / 255.0f,151 / 255.0f });
}
