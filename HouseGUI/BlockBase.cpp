#include "BlockBase.h"
#include <iostream>

BlockBase::BlockBase(float s, float o) : VAO(0), VBO(0), EBO(0), topID(0), sideID(0), bottomID(0), shaderProgram(0), size(s), hasAlpha(false), outlineSize(0.03f) {}

BlockBase::~BlockBase() {
    Cleanup();
}

void BlockBase::Init(const char* t, const char* si, const char* b) {
    LoadTexture(t, topID);
    LoadTexture(si, sideID);
    LoadTexture(b, bottomID);
    SetupShaders();
    SetupBuffers();
}

void BlockBase::Init(const std::string& tex) {
    LoadTexture(tex.c_str(), topID);
    sideID = topID;
    bottomID = topID;
    SetupShaders();
    SetupBuffers();
}

void BlockBase::Draw(const glm::mat4& view, const glm::mat4& proj, const glm::mat4& model,
    const glm::mat4& lightSpaceMatrix, const glm::vec3& lightDir,
    const glm::vec3& lightColor, const glm::vec3& viewPos, GLuint shadowMap) {
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightDir"), 1, glm::value_ptr(lightDir));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(viewPos));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, topID);
    glUniform1i(glGetUniformLocation(shaderProgram, "topTexture"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, sideID);
    glUniform1i(glGetUniformLocation(shaderProgram, "sideTexture"), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, bottomID);
    glUniform1i(glGetUniformLocation(shaderProgram, "bottomTexture"), 2);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glUniform1i(glGetUniformLocation(shaderProgram, "shadowMap"), 3);

    glUniform1f(glGetUniformLocation(shaderProgram, "outlineSize"), outlineSize);

    if (hasAlpha) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    if (hasAlpha)
        glDisable(GL_BLEND);
}

const char* BlockBase::VertexShaderSrc() {
    return "#version 330 core\n"
        "layout(location=0) in vec3 aPos;\n"
        "layout(location=1) in vec2 aTex;\n"
        "layout(location=2) in vec3 aNormal;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "uniform mat4 lightSpaceMatrix;\n"
        "out vec2 TexCoord;\n"
        "out vec3 FragPos;\n"
        "out vec3 Normal;\n"
        "out vec4 FragPosLightSpace;\n"
        "void main(){\n"
        "   FragPos = vec3(model * vec4(aPos, 1.0));\n"
        "   Normal = mat3(transpose(inverse(model))) * aNormal;\n"
        "   TexCoord = aTex;\n"
        "   FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);\n"
        "   gl_Position = projection * view * vec4(FragPos, 1.0);\n"
        "}\n";
}

const char* BlockBase::FragmentShaderSrc() {
    return "#version 330 core\n"
        "in vec2 TexCoord;\n"
        "in vec3 FragPos;\n"
        "in vec3 Normal;\n"
        "in vec4 FragPosLightSpace;\n"
        "out vec4 FragColor;\n"
        "uniform sampler2D topTexture;\n"
        "uniform sampler2D sideTexture;\n"
        "uniform sampler2D bottomTexture;\n"
        "uniform sampler2D shadowMap;\n"
        "uniform float outlineSize;\n"
        "uniform vec3 lightDir;\n"
        "uniform vec3 lightColor;\n"
        "uniform vec3 viewPos;\n"
        "float ShadowCalculation(vec4 fragPosLightSpace){\n"
        "    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;\n"
        "    projCoords = projCoords * 0.5 + 0.5;\n"
        "    float closestDepth = texture(shadowMap, projCoords.xy).r;\n"
        "    float currentDepth = projCoords.z;\n"
        "    float shadow = 0.0;\n"
        "    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);\n"
        "    for(int x = -1; x <= 1; ++x){\n"
        "       for(int y = -1; y <= 1; ++y){\n"
        "          float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;\n"
        "          shadow += currentDepth - 0.005 > pcfDepth ? 1.0 : 0.0;\n"
        "       }\n"
        "    }\n"
        "    shadow /= 9.0;\n"
        "    if(projCoords.z > 1.0) shadow = 0.0;\n"
        "    return shadow;\n"
        "}\n"
        "void main(){\n"
        "    vec4 texColor;\n"
        "    if(abs(Normal.y) > 0.9) { texColor = texture(topTexture, TexCoord); }\n"
        "    else if(Normal.y < -0.9) { texColor = texture(bottomTexture, TexCoord); }\n"
        "    else { texColor = texture(sideTexture, TexCoord); }\n"
        "    if(TexCoord.x < outlineSize || TexCoord.x > 1.0 - outlineSize || TexCoord.y < outlineSize || TexCoord.y > 1.0 - outlineSize)\n"
        "        texColor = vec4(0,0,0,1);\n"
        "    if(texColor.a < 0.1) discard;\n"
        "    vec3 norm = normalize(Normal);\n"
        "    vec3 lightDirection = normalize(-lightDir);\n"
        "    float diff = max(dot(norm, lightDirection), 0.0);\n"
        "    vec3 diffuse = diff * lightColor;\n"
        "    vec3 viewDir = normalize(viewPos - FragPos);\n"
        "    vec3 reflectDir = reflect(-lightDirection, norm);\n"
        "    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);\n"
        "    vec3 specular = spec * lightColor;\n"
        "    vec3 ambient = 0.2 * lightColor;\n"
        "    float shadow = ShadowCalculation(FragPosLightSpace);\n"
        "    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);\n"
        "    vec3 result = texColor.rgb * lighting;\n"
        "    FragColor = vec4(result, texColor.a);\n"
        "}\n";
}

void BlockBase::SetupShaders() {
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    const char* vsrc = VertexShaderSrc();
    glShaderSource(vs, 1, &vsrc, NULL);
    glCompileShader(vs);
    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fsrc = FragmentShaderSrc();
    glShaderSource(fs, 1, &fsrc, NULL);
    glCompileShader(fs);
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);
}

void BlockBase::SetupBuffers() {
    float s = size;
    float v[] = {
        -s, -s, -s, 0, 0, 0, 0, -1,
         s, -s, -s, 1, 0, 0, 0, -1,
         s,  s, -s, 1, 1, 0, 0, -1,
        -s,  s, -s, 0, 1, 0, 0, -1,
        -s, -s,  s, 0, 0, 0, 0, 1,
         s, -s,  s, 1, 0, 0, 0, 1,
         s,  s,  s, 1, 1, 0, 0, 1,
        -s,  s,  s, 0, 1, 0, 0, 1,
        -s,  s, -s, 0, 0, 0, 1, 0,
         s,  s, -s, 1, 0, 0, 1, 0,
         s,  s,  s, 1, 1, 0, 1, 0,
        -s,  s,  s, 0, 1, 0, 1, 0,
        -s, -s, -s, 0, 0, 0, -1, 0,
         s, -s, -s, 1, 0, 0, -1, 0,
         s, -s,  s, 1, 1, 0, -1, 0,
        -s, -s,  s, 0, 1, 0, -1, 0,
        -s, -s, -s, 0, 0, -1, 0, 0,
        -s,  s, -s, 0, 1, -1, 0, 0,
        -s,  s,  s, 1, 1, -1, 0, 0,
        -s, -s,  s, 1, 0, -1, 0, 0,
         s, -s, -s, 0, 0, 1, 0, 0,
         s,  s, -s, 0, 1, 1, 0, 0,
         s,  s,  s, 1, 1, 1, 0, 0,
         s, -s,  s, 1, 0, 1, 0, 0
    };
    unsigned int i[] = {
         0,  1,  2,  2,  3,  0,
         4,  5,  6,  6,  7,  4,
         8,  9, 10, 10, 11,  8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(i), i, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void BlockBase::LoadTexture(const char* path, unsigned int& texID) {
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
    }
    else {
        std::cout << "Failed to load " << path << "\n";
    }
    stbi_image_free(data);
}

void BlockBase::Cleanup() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
    if (topID) glDeleteTextures(1, &topID);
    if (sideID && sideID != topID) glDeleteTextures(1, &sideID);
    if (bottomID && bottomID != topID && bottomID != sideID) glDeleteTextures(1, &bottomID);
    if (shaderProgram) glDeleteProgram(shaderProgram);
}
