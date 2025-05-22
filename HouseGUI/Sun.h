#pragma once
#include <glm/glm.hpp>

class Sun {
public:
    glm::vec3 position;
    glm::vec3 color;
    float intensity;

    Sun() {
        position = glm::vec3(10.0f, 25.0f, 10.0f);
        color = glm::vec3(10.0f, 10.0f, 10.9f);
        intensity = 10.0f;
    }
    Sun(const glm::vec3& pos, const glm::vec3& c, float i) {
        position = pos;
        color = c;
        intensity = i;
    }
};
