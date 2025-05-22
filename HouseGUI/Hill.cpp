    #include "Hill.h"
    #include <glad/glad.h>
    #include <glm/gtc/type_ptr.hpp>
    #include <vector>
    #include <cmath>
    #include "stb/stb_image.h"

    Hill::Hill(float bs, float h, int seg, float exp, float sq)
        : baseSize(bs), height(h), segments(seg), exponent(exp), squareSize(sq / 0.64f),
        VAO(0), VBO(0), EBO(0), shader(0), textureID(0), indexCount(0)
    {
    }

    Hill::~Hill() {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);
        if (shader) glDeleteProgram(shader);
        if (textureID) glDeleteTextures(1, &textureID);
    }

    void Hill::Init() {
        generateMesh();

        const char* vs = R"(
    #version 330 core
    layout(location=0) in vec3 aPos;
    layout(location=1) in vec3 aNormal;
    layout(location=2) in vec2 aTex;
    uniform mat4 model, view, projection, lightSpaceMatrix;
    out vec3 FragPos, Normal;
    out vec4 FragPosLightSpace;
    out vec2 TexCoord;
    void main(){
        FragPos = vec3(model * vec4(aPos,1.0));
        Normal  = mat3(transpose(inverse(model))) * aNormal;
        FragPosLightSpace = lightSpaceMatrix * vec4(FragPos,1.0);
        TexCoord = aTex;
        gl_Position = projection * view * vec4(FragPos,1.0);
    }
    )";

        const char* fs = R"(
    #version 330 core
    in vec3 FragPos, Normal;
    in vec4 FragPosLightSpace;
    in vec2 TexCoord;
    uniform sampler2D shadowMap, hillTexture;
    uniform vec3 lightDir, lightColor, viewPos;
    uniform float outlineSize;
    out vec4 FragColor;
    float ShadowCalculation(vec4 fpos){
        vec3 pc = fpos.xyz / fpos.w;
        pc = pc * 0.5 + 0.5;
        float closest = texture(shadowMap, pc.xy).r;
        float current = pc.z;
        float shadow = 0.0;
        vec2 ts = 1.0 / textureSize(shadowMap,0);
        for(int x=-1;x<=1;x++) for(int y=-1;y<=1;y++){
            float p = texture(shadowMap, pc.xy + vec2(x,y)*ts).r;
            shadow += (current - 0.005 > p) ? 1.0 : 0.0;
        }
        shadow /= 9.0;
        if(pc.z > 1.0) shadow = 0.0;
        return shadow;
    }
    void main(){
        vec2 f = fract(TexCoord);
        if(f.x < outlineSize || f.x > 1.0 - outlineSize ||
           f.y < outlineSize || f.y > 1.0 - outlineSize){
            FragColor = vec4(0,0,0,1);
            return;
        }
        vec3 n = normalize(Normal);
        vec3 ld = normalize(-lightDir);
        float diff = max(dot(n,ld), 0.0);
        vec3 difC = diff * lightColor;
        vec3 viewD = normalize(viewPos - FragPos);
        vec3 refD = reflect(-ld,n);
        float spec = pow(max(dot(viewD,refD),0.0),32.0);
        vec3 specC = spec * lightColor;
        vec3 amb = 0.1 * lightColor;
        float sh = ShadowCalculation(FragPosLightSpace);
        vec3 light = amb + (1.0 - sh)*(difC + specC);
        vec4 tex = texture(hillTexture, TexCoord);
        FragColor = vec4(tex.rgb * light, tex.a);
    }
    )";

        shader = createProgram(vs, fs);

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        int w, h, n;
        unsigned char* data = stbi_load("textures/grass_carried.png", &w, &h, &n, 4);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glUseProgram(shader);
        glUniform1f(glGetUniformLocation(shader, "outlineSize"), 0.03f);
    }

    void Hill::generateMesh() {
        std::vector<glm::vec3> verts;
        std::vector<glm::vec3> norms;
        std::vector<glm::vec2> uvs;
        std::vector<unsigned int> idx;
        float half = baseSize * 0.5f;
        for (int i = 0; i <= segments; i++) {
            float x = -half + baseSize * float(i) / segments;
            float t = (x + half) / baseSize;
            float y = height * (1.0f - pow(t, exponent));
            float dy = (i == 0) ? 1e5f : -height * exponent / baseSize * pow(t, exponent - 1);
            glm::vec3 n = (i == 0) ? glm::vec3(-1, 0, 0) : glm::normalize(glm::vec3(-dy, 1, 0));
            for (int j = 0; j <= segments; j++) {
                float z = -half + baseSize * float(j) / segments;
                verts.emplace_back(x, y, z);
                norms.push_back(n);
                uvs.emplace_back(float(i) * squareSize, float(j) * squareSize);
            }
        }
        int topCount = (segments + 1) * (segments + 1);
        for (int i = 0; i < segments; i++) {
            for (int j = 0; j < segments; j++) {
                int a = i * (segments + 1) + j;
                int b = (i + 1) * (segments + 1) + j;
                idx.push_back(a);
                idx.push_back(b);
                idx.push_back(a + 1);
                idx.push_back(a + 1);
                idx.push_back(b);
                idx.push_back(b + 1);
            }
        }
        for (int i = 0; i <= segments; i++) {
            float x = -half + baseSize * float(i) / segments;
            for (int j = 0; j <= segments; j++) {
                float z = -half + baseSize * float(j) / segments;
                verts.emplace_back(x, 0.0f, z);
                norms.emplace_back(0.0f, -1.0f, 0.0f);
                uvs.emplace_back(float(i) * squareSize, float(j) * squareSize);
            }
        }
        int baseOffset = topCount;
        for (int i = 0; i < segments; i++) {
            for (int j = 0; j < segments; j++) {
                int a = baseOffset + i * (segments + 1) + j;
                int b = baseOffset + (i + 1) * (segments + 1) + j;
                idx.push_back(a);
                idx.push_back(b);
                idx.push_back(a + 1);
                idx.push_back(a + 1);
                idx.push_back(b);
                idx.push_back(b + 1);
            }
        }
        int wallOffset = topCount + topCount;
        for (int i = 0; i < segments; i++) {
            float x1 = -half + baseSize * float(i) / segments;
            float x2 = -half + baseSize * float(i + 1) / segments;
            float t1 = (x1 + half) / baseSize;
            float t2 = (x2 + half) / baseSize;
            float y1 = height * (1.0f - pow(t1, exponent));
            float y2 = height * (1.0f - pow(t2, exponent));
            verts.emplace_back(x1, y1, -half);
            norms.emplace_back(0, 0, -1);
            uvs.emplace_back(float(i) * squareSize, 0);
            verts.emplace_back(x2, y2, -half);
            norms.emplace_back(0, 0, -1);
            uvs.emplace_back(float(i + 1) * squareSize, 0);
            verts.emplace_back(x1, 0, -half);
            norms.emplace_back(0, 0, -1);
            uvs.emplace_back(float(i) * squareSize, 1 * squareSize);
            verts.emplace_back(x2, 0, -half);
            norms.emplace_back(0, 0, -1);
            uvs.emplace_back(float(i + 1) * squareSize, 1 * squareSize);
            int off = wallOffset + i * 4;
            idx.push_back(off);
            idx.push_back(off + 2);
            idx.push_back(off + 1);
            idx.push_back(off + 1);
            idx.push_back(off + 2);
            idx.push_back(off + 3);
        }
        wallOffset += segments * 4;
        for (int i = 0; i < segments; i++) {
            float x1 = -half + baseSize * float(i) / segments;
            float x2 = -half + baseSize * float(i + 1) / segments;
            float t1 = (x1 + half) / baseSize;
            float t2 = (x2 + half) / baseSize;
            float y1 = height * (1.0f - pow(t1, exponent));
            float y2 = height * (1.0f - pow(t2, exponent));
            verts.emplace_back(x2, y2, half);
            norms.emplace_back(0, 0, 1);
            uvs.emplace_back(float(i + 1) * squareSize, 0);
            verts.emplace_back(x1, y1, half);
            norms.emplace_back(0, 0, 1);
            uvs.emplace_back(float(i) * squareSize, 0);
            verts.emplace_back(x2, 0, half);
            norms.emplace_back(0, 0, 1);
            uvs.emplace_back(float(i + 1) * squareSize, 1 * squareSize);
            verts.emplace_back(x1, 0, half);
            norms.emplace_back(0, 0, 1);
            uvs.emplace_back(float(i) * squareSize, 1 * squareSize);
            int off = wallOffset + i * 4;
            idx.push_back(off);
            idx.push_back(off + 2);
            idx.push_back(off + 1);
            idx.push_back(off + 1);
            idx.push_back(off + 2);
            idx.push_back(off + 3);
        }
        wallOffset += segments * 4;
        for (int j = 0; j < segments; j++) {
            float z1 = -half + baseSize * float(j) / segments;
            float z2 = -half + baseSize * float(j + 1) / segments;
            verts.emplace_back(-half, height, z1);
            norms.emplace_back(-1, 0, 0);
            uvs.emplace_back(0, float(j) * squareSize);
            verts.emplace_back(-half, height, z2);
            norms.emplace_back(-1, 0, 0);
            uvs.emplace_back(0, float(j + 1) * squareSize);
            verts.emplace_back(-half, 0, z1);
            norms.emplace_back(-1, 0, 0);
            uvs.emplace_back(1 * squareSize, float(j) * squareSize);
            verts.emplace_back(-half, 0, z2);
            norms.emplace_back(-1, 0, 0);
            uvs.emplace_back(1 * squareSize, float(j + 1) * squareSize);
            int off = wallOffset + j * 4;
            idx.push_back(off);
            idx.push_back(off + 2);
            idx.push_back(off + 1);
            idx.push_back(off + 1);
            idx.push_back(off + 2);
            idx.push_back(off + 3);
        }
        indexCount = static_cast<GLuint>(idx.size());
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        size_t vb = verts.size() * sizeof(glm::vec3);
        size_t nb = norms.size() * sizeof(glm::vec3);
        size_t ub = uvs.size() * sizeof(glm::vec2);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vb + nb + ub, nullptr, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vb, verts.data());
        glBufferSubData(GL_ARRAY_BUFFER, vb, nb, norms.data());
        glBufferSubData(GL_ARRAY_BUFFER, vb + nb, ub, uvs.data());
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned), idx.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(vb));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)(vb + nb));
        glBindVertexArray(0);
    }

    GLuint Hill::compileShader(const char* src, GLenum type) {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        return s;
    }

    GLuint Hill::createProgram(const char* vs, const char* fs) {
        GLuint v = compileShader(vs, GL_VERTEX_SHADER);
        GLuint f = compileShader(fs, GL_FRAGMENT_SHADER);
        GLuint p = glCreateProgram();
        glAttachShader(p, v);
        glAttachShader(p, f);
        glLinkProgram(p);
        glDeleteShader(v);
        glDeleteShader(f);
        return p;
    }

    void Hill::Draw(const glm::mat4& view,
        const glm::mat4& proj,
        const glm::mat4& model,
        const glm::mat4& lightSpaceMatrix,
        const glm::vec3& lightDir,
        const glm::vec3& lightColor,
        const glm::vec3& viewPos,
        GLuint shadowMap)
    {
        glUseProgram(shader);
        glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(glGetUniformLocation(shader, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
        glUniform3fv(glGetUniformLocation(shader, "lightDir"), 1, glm::value_ptr(lightDir));
        glUniform3fv(glGetUniformLocation(shader, "lightColor"), 1, glm::value_ptr(lightColor));
        glUniform3fv(glGetUniformLocation(shader, "viewPos"), 1, glm::value_ptr(viewPos));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glUniform1i(glGetUniformLocation(shader, "hillTexture"), 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, shadowMap);
        glUniform1i(glGetUniformLocation(shader, "shadowMap"), 1);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }