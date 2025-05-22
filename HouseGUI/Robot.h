// Robot.h
#ifndef ROBOT_H
#define ROBOT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

enum class HeadFace { Front, Back, Left, Right, Top, Bottom };

class Robot {
public:
    Robot(int width, int height, float scale = 1.0f);
    ~Robot();
    void Update(float dt, GLFWwindow* window);
    void Draw(const glm::mat4& view, const glm::mat4& proj);
    glm::vec3 Position;
    float Yaw;
private:
    float uniformScale;
    static constexpr float Gravity = -9.8f;
    static constexpr float BaseOffset = 0.6f;
    static constexpr float MoveSpeed = 3.0f;
    static constexpr float TurnSpeed = 0.0f;
    glm::vec3 Velocity;
    GLuint shader, VAO, VBO, EBO, headTextures[6], headOverlayTexture;
    GLint locModel, locView, locProj, useTex, locColor;
    float walkCycle;
    void init();
    void loadHeadTextures();
    void loadHeadOverlayTexture();
    GLuint compile(const char* src, GLenum type);
    void drawHead(const glm::mat4& model, const glm::vec3& offset, const glm::vec3& scale, HeadFace face);
    void drawCubeColor(const glm::mat4& model, const glm::vec3& offset, const glm::vec3& scale, const glm::vec3& color);
    void drawLimb(const glm::mat4& parent, const glm::vec3& offset, float yaw, float pitch, float elbow,
        const glm::vec3& upperSize, const glm::vec3& lowerSize,
        const glm::vec3& upperColor, const glm::vec3& lowerColor);
};

#endif
