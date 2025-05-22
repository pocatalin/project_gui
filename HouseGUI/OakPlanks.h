#ifndef OAKPLANKS_H
#define OAKPLANKS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb/stb_image.h"
#include <iostream>

class OakPlanks {
public:
    OakPlanks(float size);
    ~OakPlanks();
    void Draw(const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model);

private:
    unsigned int VBO, VAO, EBO;
    unsigned int texture;
    void setupBuffers(float size);
    void setupTexture();
};

#endif 