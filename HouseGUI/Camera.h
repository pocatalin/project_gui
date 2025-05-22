#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT };

class Camera {
public:
    glm::vec3 Position, Front, Up, Right, WorldUp;
    float Yaw, Pitch, MovementSpeed, MouseSensitivity, Zoom;

    Camera(glm::vec3 pos = glm::vec3(0, 0, 0)) {
        Position = pos; WorldUp = glm::vec3(0, 1, 0); Yaw = -90; Pitch = 0; Front = glm::vec3(0, 0, -1);
        MovementSpeed = 2.5f; MouseSensitivity = 0.1f; Zoom = 45; UpdateCameraVectors();
    }

    glm::mat4 GetViewMatrix() {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void ProcessKeyboard(Camera_Movement direction, float velocity) {
        glm::vec3 newPos = Position;
        if (direction == FORWARD)newPos += Front * velocity;
        if (direction == BACKWARD)newPos -= Front * velocity;
        if (direction == LEFT)newPos -= Right * velocity;
        if (direction == RIGHT)newPos += Right * velocity;
        if (!CheckCollision(newPos)) {
            Position = newPos;
        }
    }

    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;
        Yaw += xoffset;
        Pitch += yoffset;
        if (constrainPitch) {
            if (Pitch > 89)Pitch = 89;
            if (Pitch < -89)Pitch = -89;
        }
        UpdateCameraVectors();
    }

    void ProcessMouseScroll(float yoffset) {
        Zoom -= yoffset;
        if (Zoom < 1)Zoom = 1;
        if (Zoom > 45)Zoom = 45;
    }

private:
    void UpdateCameraVectors() {
        glm::vec3 f;
        f.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        f.y = sin(glm::radians(Pitch));
        f.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(f);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }

    bool CheckCollision(const glm::vec3& newPos) {
        float min = -50, max = 50;
        if (newPos.x<min || newPos.x>max || newPos.y < -10 || newPos.y>50 || newPos.z<min || newPos.z>max)return true;
        return false;
    }
};
