#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Camera.h"
#include "OakLog.h"
#include "GrassBlock.h"
#include "Stairs.h"
#include "Flower.h"
#include "Leaves.h"
#include "Glass.h"
#include "Door.h"
#include "Robot.h"
#include "Hill.h"
#include <vector>
#include <algorithm>

Camera camera(glm::vec3(0.0f, 2.0f, 10.0f));
float lastX = 400.0f, lastY = 300.0f;
bool firstMouse = true;

OakLog* oakLogCube = nullptr;
GrassBlock* grassBlock = nullptr;
Stairs* stairs = nullptr;
Leaves* leaves = nullptr;
Panel* glassPanel = nullptr;
Door* door = nullptr;
Flower* flowers[5] = { nullptr };
Robot* robot = nullptr;
Hill* hill = nullptr;
GLuint sceneShader = 0;
const float TurnSpeed = 90.0f;

glm::vec3 dirLightColor(2.0f, 2.0f, 2.0f);
glm::vec3 cornerLightColor(1.0f, 5.0f, 1.0f);

const float hillBaseRadius = 9.0f;
const float grassPlaneSize = 25.0f;
const float blockSize = 0.2f;
const float spacing = blockSize * 2.0f;
const float planeOffset = (grassPlaneSize - 1.0f) * spacing * 0.5f;
const float gravity = -9.8f;
const float robotBaseOffset = 0.6f;

const float hillMaxHeight = 1.0f;
const float hillExponent = 3.0f;
glm::vec3 hillCenter(0.0f, 0.08f, planeOffset + hillBaseRadius - 17.8f);

glm::mat4 projection;
glm::vec3 robotPos(2.0f, 2.0f, 0.0f);
float robotYaw = 0.0f;
const float robotSpeed = 3.0f;

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = (float)xposIn, ypos = (float)yposIn;
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoff = xpos - lastX, yoff = lastY - ypos;
    lastX = xpos; lastY = ypos;
    camera.ProcessMouseMovement(xoff, yoff, true);
}

void scroll_callback(GLFWwindow* window, double, double yoff) {
    camera.ProcessMouseScroll((float)yoff);
}

void processInput(GLFWwindow* w, float dt) {
    if (glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(w, true);
    float speed = 5.0f * dt;
    if (glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, speed);
    if (glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, speed);
    if (glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, speed);
    if (glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, speed);
    if (glfwGetKey(w, GLFW_KEY_SPACE) == GLFW_PRESS) camera.Position.y += speed;
    if (glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camera.Position.y -= speed;
}


const float groundY = 0.2f;

float getTerrainHeight(float x, float z) {
    return groundY;
}

void updateRobot(float dt, GLFWwindow* w) {
    static glm::vec3 vel(0.0f);
    vel.y += gravity * dt;
    robotPos.y += vel.y * dt;

    if (robotPos.y <= groundY) {
        robotPos.y = groundY;
        vel.y = 0.0f;

        if (glfwGetKey(w, GLFW_KEY_LEFT) == GLFW_PRESS)  robotYaw += TurnSpeed * dt;
        if (glfwGetKey(w, GLFW_KEY_RIGHT) == GLFW_PRESS)  robotYaw -= TurnSpeed * dt;

        glm::vec3 dir(0.0f);
        if (glfwGetKey(w, GLFW_KEY_UP) == GLFW_PRESS)  dir += glm::vec3(sinf(glm::radians(robotYaw)), 0, cosf(glm::radians(robotYaw)));
        if (glfwGetKey(w, GLFW_KEY_DOWN) == GLFW_PRESS)  dir -= glm::vec3(sinf(glm::radians(robotYaw)), 0, cosf(glm::radians(robotYaw)));

        if (glm::length(dir) > 0.0f) {
            vel.x = dir.x * robotSpeed;
            vel.z = dir.z * robotSpeed;
        }
        else {
            vel.x = vel.z = 0.0f;
        }
    }

    robotPos.x += vel.x * dt;
    robotPos.z += vel.z * dt;
}




void addLayer(const std::vector<std::vector<int>>& layer, const glm::vec3& centerPos, float height,
    const glm::mat4& view, const glm::mat4& proj, const glm::mat4& lightSpaceMatrix,
    const glm::vec3& lightDir, const glm::vec3& lightColor, const glm::vec3& viewPos,
    GLuint shadowMap) {
    int s = (int)layer.size();
    if (s > 25 || s % 2 == 0) return;
    for (int i = 0; i < s; i++) for (int j = 0; j < s; j++) {
        glm::vec3 position = centerPos + glm::vec3(i * spacing - (s - 1) * spacing * 0.5f, height, j * spacing - (s - 1) * spacing * 0.5f);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
        if (layer[i][j] == 2) oakLogCube->Draw(view, proj, model, lightSpaceMatrix, lightDir, lightColor, viewPos, shadowMap);
        else if (layer[i][j] == 3) stairs->Draw(view, proj, model, lightSpaceMatrix, lightDir, lightColor, viewPos, shadowMap);
        else if (layer[i][j] == 4) leaves->Draw(view, proj, model, lightSpaceMatrix, lightDir, lightColor, viewPos, shadowMap);
    }
}

void createGrassLayer(const glm::mat4& view, const glm::mat4& proj, const glm::mat4& lightSpaceMatrix,
    const glm::vec3& lightDir, const glm::vec3& lightColor, const glm::vec3& viewPos,
    GLuint shadowMap) {
    int size = (int)grassPlaneSize;
    for (int i = 0; i < size; i++) for (int j = 0; j < size; j++) {
        if (i >= 11 && i <= 13 && j >= 11 && j <= 13) continue;
        if (i == 12 && j == 14) continue;
        glm::vec3 position(i * spacing - planeOffset, 0.0f, j * spacing - planeOffset);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
        grassBlock->Draw(view, proj, model, lightSpaceMatrix, lightDir, lightColor, viewPos, shadowMap);
    }
}

void createFlowers(const glm::mat4& view, const glm::mat4& proj, const glm::mat4& lightSpaceMatrix,
    const glm::vec3& lightDir, const glm::vec3& lightColor, const glm::vec3& viewPos,
    GLuint shadowMap) {
    const char* ft[5] = { "flower_blue_orchid.png","flower_dandelion.png","flower_tulip_white.png","flower_oxeye_daisy.png","flower_rose.png" };
    struct FlowerPos { int x, z, ti; } fp[] = {
        {22,4,0},{3,3,1},{7,5,2},{19,4,3},{15,6,4},{5,2,0},{17,3,1},{9,4,2},{13,5,3},{21,6,4},
        {4,8,0},{20,9,1},{8,7,2},{16,8,3},{12,7,4},{6,9,0},{18,7,1},{10,8,2},{14,9,3},{22,8,4},
        {3,15,0},{21,16,1},{7,17,2},{19,15,3},{15,16,4},{5,18,0},{17,19,1},{9,20,2},{13,18,3},{21,20,4},
        {2,6,0},{4,12,1},{3,9,2},{2,15,3},{4,18,4},{3,21,0},{2,12,1},{4,15,2},{3,18,3},{2,9,4},
        {20,6,0},{22,12,1},{21,9,2},{20,15,3},{22,18,4},{21,21,0},{20,12,1},{22,15,2},{21,18,3},{20,9,4},
        {8,22,0},{16,22,1},{12,21,2},{14,20,3},{10,19,4},{6,20,0},{18,21,1},{11,22,2},{15,19,3},{7,21,4}
    };
    static Flower* fl[5] = { nullptr };
    if (!fl[0]) for (int i = 0; i < 5; i++) { fl[i] = new Flower(0.1f, ft[i]); fl[i]->Init(); }
    for (auto& p : fp) {
        float xOff = (((p.x * 13 + p.z * 17) % 7) / 10.0f - 0.35f) * 0.4f;
        float zOff = (((p.x * 19 + p.z * 23) % 7) / 10.0f - 0.35f) * 0.4f;
        float yOff = (((p.x * 29 + p.z * 31) % 5) / 10.0f) * 0.15f;
        glm::vec3 pos(p.x * spacing - planeOffset + xOff, 0.5f + yOff, p.z * spacing - planeOffset + zOff);
        glm::mat4 m = glm::translate(glm::mat4(1.0f), pos);
        glm::vec3 toCam = glm::normalize(camera.Position - pos);
        float a = atan2(toCam.x, toCam.z);
        m = glm::rotate(m, a, glm::vec3(0, 1, 0));
        m = glm::rotate(m, glm::pi<float>(), glm::vec3(1, 0, 0));
        fl[p.ti]->Draw(view, proj, m, lightSpaceMatrix, lightDir, lightColor, viewPos, shadowMap);
    }
}



void createGlassPanels(const glm::mat4& view, const glm::mat4& proj, const glm::mat4& lightSpaceMatrix,
    const glm::vec3& lightDir, const glm::vec3& lightColor, const glm::vec3& viewPos,
    GLuint shadowMap) {
    glm::vec3 pp[] = {
        {1,0.4f,1},{1,0.4f,0},{1,0.4f,-1},
        {-1,0.4f,1},{-1,0.4f,0},{-1,0.4f,-1},
        {0,0.4f,1},{-0.5f,0.4f,1},{0.5f,0.4f,1},
        {0,0.4f,-1},{-0.5f,0.4f,-1},{0.5f,0.4f,-1}
    };
    for (auto& p : pp) {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), p);
        if (p.x == 1 || p.x == -1) m = glm::rotate(m, glm::radians(90.0f), glm::vec3(0, 1, 0));
        glassPanel->Draw(view, proj, m, lightSpaceMatrix, lightDir, lightColor, viewPos, shadowMap);
    }
}

void createDoor(const glm::mat4& view, const glm::mat4& proj, const glm::mat4& lightSpaceMatrix,
    const glm::vec3& lightDir, const glm::vec3& lightColor, const glm::vec3& viewPos,
    GLuint shadowMap) {
    static Door* d = nullptr;
    if (!d) { d = new Door(0.5f); d->Init(); }
    float off = (grassPlaneSize - 1) * spacing * 0.5f;
    glm::vec3 pos(12 * spacing - off, 0.2f, 14 * spacing - off);
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos);
    d->Draw(view, proj, m, lightSpaceMatrix, lightDir, lightColor, viewPos, shadowMap);
}

void createTree(const glm::mat4& view, const glm::mat4& proj, const glm::vec3& pos, int height,
    const glm::mat4& lightSpaceMatrix, const glm::vec3& lightDir,
    const glm::vec3& lightColor, const glm::vec3& viewPos, GLuint shadowMap) {
    for (int i = 0; i < height; i++) {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, i * 0.4f, 0.0f));
        oakLogCube->Draw(view, proj, m, lightSpaceMatrix, lightDir, lightColor, viewPos, shadowMap);
    }
    auto drawL = [&](const glm::vec3& p) {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), p);
        leaves->Draw(view, proj, m, lightSpaceMatrix, lightDir, lightColor, viewPos, shadowMap);
        };
    for (int x = -2; x <= 2; x++)
        for (int z = -2; z <= 2; z++)
            drawL(pos + glm::vec3(x * 0.4f, (height - 3) * 0.4f, z * 0.4f));
    for (int x = -2; x <= 2; x++)
        for (int z = -2; z <= 2; z++)
            drawL(pos + glm::vec3(x * 0.4f, (height - 2) * 0.4f, z * 0.4f));
    for (int x = -1; x <= 1; x++)
        for (int z = -1; z <= 1; z++)
            drawL(pos + glm::vec3(x * 0.4f, (height - 1) * 0.4f, z * 0.4f));
    drawL(pos + glm::vec3(0.0f, height * 0.4f, 0.0f));
}

void createAllTrees(const glm::mat4& view, const glm::mat4& proj, const glm::mat4& lightSpaceMatrix,
    const glm::vec3& lightDir, const glm::vec3& lightColor, const glm::vec3& viewPos,
    GLuint shadowMap) {
    float off = (grassPlaneSize - 1) * spacing * 0.5f;
    glm::ivec3 tp[] = {
        { 2,  2, 7},
        { 2, 22, 7},
        {22,  2, 7},
        {22, 22, 7},
        {22, 12, 7},
        { 5,-15, 12},
        { 10,-5, 10},
        { 15,10, 12},
        { 20,-12, 12},

    };
    for (auto& t : tp) {
        glm::vec3 pos(t.x * spacing - off, 0.0f, t.y * spacing - off);
        createTree(view, proj, pos, t.z, lightSpaceMatrix, lightDir, lightColor, viewPos, shadowMap);
    }
}

void createLeaves(const glm::mat4& view, const glm::mat4& proj, const glm::mat4& lightSpaceMatrix,
    const glm::vec3& lightDir, const glm::vec3& lightColor, const glm::vec3& viewPos,
    GLuint shadowMap) {
    glm::vec3 lp[] = {
        {0.0f,1.2f,0.0f},{0.0f,1.4f,0.0f},{0.0f,1.6f,0.0f},
        {0.4f,1.2f,0.0f},{-0.4f,1.2f,0.0f},{0.0f,1.2f,0.4f},{0.0f,1.2f,-0.4f},
        {0.4f,1.2f,0.4f},{-0.4f,1.2f,0.4f},{0.4f,1.2f,-0.4f},{-0.4f,1.2f,-0.4f},
        {0.4f,1.4f,0.0f},{-0.4f,1.4f,0.0f},{0.0f,1.4f,0.4f},{0.0f,1.4f,-0.4f},
        {0.4f,1.6f,0.0f},{-0.4f,1.6f,0.0f},{0.0f,1.6f,0.4f},{0.0f,1.6f,-0.4f}
    };
    for (auto& p : lp) {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), p);
        leaves->Draw(view, proj, m, lightSpaceMatrix, lightDir, lightColor, viewPos, shadowMap);
    }
}
void createHouse(const glm::mat4& view, const glm::mat4& proj, const glm::mat4& lightSpaceMatrix,
    const glm::vec3& lightDir, const glm::vec3& lightColor, const glm::vec3& viewPos,
    GLuint shadowMap) {
    float off = (grassPlaneSize - 1) * spacing * 0.5f;
    glm::vec3 base(12 * spacing - off, -0.2f, 12 * spacing - off);
    std::vector<std::vector<int>> l1 = { {2,2,2},{2,2,2},{2,2,2} };
    addLayer(l1, base, 0.2f, view, proj, lightSpaceMatrix, lightDir, lightColor, viewPos, shadowMap);
    for (int h = 0; h < 4; h++) {
        float y = h * 0.4f + 0.2f;
        for (int x = -2; x <= 2; x++) for (int z = -2; z <= 2; z++) {
            if (x > -2 && x < 2 && z > -2 && z < 2) continue;
            if (h < 2 && z == 2 && x == 0) continue;
            glm::mat4 m = glm::translate(glm::mat4(1.0f), base + glm::vec3(x * spacing, y, z * spacing));
            if ((x == -2 || x == 2) && (z == -2 || z == 2)) oakLogCube->Draw(view, proj, m, lightSpaceMatrix, lightDir, lightColor, viewPos, shadowMap);
            else if (h == 2) {
                if (z == 2 && x == 0) continue;
                if (z == 2 || z == -2) { m = glm::rotate(m, glm::radians(180.0f), glm::vec3(0, 1, 0)); glassPanel->Draw(view, proj, m, lightSpaceMatrix, lightDir, lightColor, viewPos, shadowMap); }
                else if (x == -2 || x == 2) { m = glm::rotate(m, glm::radians(90.0f), glm::vec3(0, 1, 0)); glassPanel->Draw(view, proj, m, lightSpaceMatrix, lightDir, lightColor, viewPos, shadowMap); }
            }
            else oakLogCube->Draw(view, proj, m, lightSpaceMatrix, lightDir, lightColor, viewPos, shadowMap);
        }
    }
    glm::mat4 dM = glm::translate(glm::mat4(1.0f), base + glm::vec3(0, 0.2f, 2 * spacing));
    oakLogCube->Draw(view, proj, dM, lightSpaceMatrix, lightDir, lightColor, viewPos, shadowMap);
    float rh = 3 * 0.4f + 0.6f;
    for (int x = -2; x <= 2; x++) for (int z = -2; z <= 2; z++) {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), base + glm::vec3(x * spacing, rh, z * spacing));
        if (z == -2) m = glm::rotate(m, glm::radians(270.0f), glm::vec3(0, 1, 0));
        else if (z == 2) m = glm::rotate(m, glm::radians(90.0f), glm::vec3(0, 1, 0));
        else if (x == -2) m = glm::rotate(m, glm::radians(0.0f), glm::vec3(0, 1, 0));
        else if (x == 2) m = glm::rotate(m, glm::radians(180.0f), glm::vec3(0, 1, 0));
        stairs->Draw(view, proj, m, lightSpaceMatrix, lightDir, lightColor, viewPos, shadowMap);
    }
    rh += 0.4f;
    for (int x = -1; x <= 1; x++) for (int z = -1; z <= 1; z++) {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), base + glm::vec3(x * spacing, rh, z * spacing));
        if (z == -1) m = glm::rotate(m, glm::radians(270.0f), glm::vec3(0, 1, 0));
        else if (z == 1) m = glm::rotate(m, glm::radians(90.0f), glm::vec3(0, 1, 0));
        else if (x == -1) m = glm::rotate(m, glm::radians(0.0f), glm::vec3(0, 1, 0));
        else if (x == 1) m = glm::rotate(m, glm::radians(180.0f), glm::vec3(0, 1, 0));
        stairs->Draw(view, proj, m, lightSpaceMatrix, lightDir, lightColor, viewPos, shadowMap);
    }
    rh += 0.4f;
    glm::mat4 tM = glm::translate(glm::mat4(1.0f), base + glm::vec3(0, rh, 0));
    oakLogCube->Draw(view, proj, tM, lightSpaceMatrix, lightDir, lightColor, viewPos, shadowMap);
}

void renderScene(const glm::mat4& view, const glm::mat4& proj, const glm::mat4& lightSpaceMatrix,
    const glm::vec3& lightDir, const glm::vec3& lightColor, const glm::vec3& viewPos,
    GLuint shadowMap) {
    createGrassLayer(view, proj, lightSpaceMatrix, lightDir, lightColor, viewPos, shadowMap);
    createAllTrees(view, proj, lightSpaceMatrix, lightDir, lightColor, viewPos, shadowMap);
    createHouse(view, proj, lightSpaceMatrix, lightDir, lightColor, viewPos, shadowMap);
    createFlowers(view, proj, lightSpaceMatrix, lightDir, lightColor, viewPos, shadowMap);
    createDoor(view, proj, lightSpaceMatrix, lightDir, lightColor, viewPos, shadowMap);
}

GLuint compileShader(const char* src, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    return shader;
}

GLuint createProgram(const char* vs, const char* fs) {
    GLuint vertexShader = compileShader(vs, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fs, GL_FRAGMENT_SHADER);
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}


const char* depthVertexShaderSource = "#version 330 core\nlayout(location=0) in vec3 aPos;\nuniform mat4 model;\nuniform mat4 lightSpaceMatrix;\nvoid main(){gl_Position=lightSpaceMatrix*model*vec4(aPos,1.0);}";

const char* depthFragmentShaderSource = "#version 330 core\nvoid main(){}";

const char* sceneVertexShaderSource = "#version 330 core\nlayout(location=0) in vec3 aPos;\nlayout(location=1) in vec3 aNormal;\nuniform mat4 model;\nuniform mat4 view;\nuniform mat4 projection;\nuniform mat4 lightSpaceMatrix;\nout vec3 FragPos;\nout vec3 Normal;\nout vec4 FragPosLightSpace;\nvoid main(){FragPos=vec3(model*vec4(aPos,1.0));Normal=mat3(transpose(inverse(model)))*aNormal;FragPosLightSpace=lightSpaceMatrix*vec4(FragPos,1.0);gl_Position=projection*view*vec4(FragPos,1.0);}";

const char* sceneFragmentShaderSource = "#version 330 core\nin vec3 FragPos;\nin vec3 Normal;\nin vec4 FragPosLightSpace;\nuniform sampler2D shadowMap;\nuniform vec3 lightDir;\nuniform vec3 lightColor;\nuniform vec3 cornerLightPos;\nuniform vec3 cornerLightColor;\nuniform vec3 viewPos;\nout vec4 FragColor;\nfloat ShadowCalculation(vec4 fragPosLightSpace){vec3 projCoords=fragPosLightSpace.xyz/fragPosLightSpace.w;projCoords=projCoords*0.5+0.5;float closestDepth=texture(shadowMap,projCoords.xy).r;float currentDepth=projCoords.z;float shadow=0.0;vec2 texelSize=1.0/textureSize(shadowMap,0);for(int x=-1;x<=1;x++){for(int y=-1;y<=1;y++){float pcfDepth=texture(shadowMap,projCoords.xy+vec2(x,y)*texelSize).r;shadow+=currentDepth-0.005>pcfDepth?1.0:0.0;}}shadow/=9.0;if(projCoords.z>1.0)shadow=0.0;return shadow;}void main(){vec3 norm=normalize(Normal);vec3 lightDirNorm=normalize(-lightDir);float diff=max(dot(norm,lightDirNorm),0.0);vec3 diffuse=diff*lightColor;vec3 viewDir=normalize(viewPos-FragPos);vec3 reflectDir=reflect(-lightDirNorm,norm);float spec=pow(max(dot(viewDir,reflectDir),0.0),32.0);vec3 specular=spec*lightColor;vec3 ambient=0.1*lightColor;float shadow=ShadowCalculation(FragPosLightSpace);vec3 result=(ambient+(1.0-shadow)*(diffuse+specular));vec3 cornerLightDir=normalize(cornerLightPos-FragPos);float diff2=max(dot(norm,cornerLightDir),0.0);vec3 diffuse2=diff2*cornerLightColor;vec3 reflectDir2=reflect(-cornerLightDir,norm);float spec2=pow(max(dot(viewDir,reflectDir2),0.0),32.0);vec3 specular2=spec2*cornerLightColor;result+=ambient+(diffuse2+specular2);FragColor=vec4(result,1.0);}";

int main() {
    glfwInit();
    GLFWwindow* win = glfwCreateWindow(800, 600, "Upgraded Project", nullptr, nullptr);
    if (!win) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(win);
    glfwSetCursorPosCallback(win, mouse_callback);
    glfwSetScrollCallback(win, scroll_callback);
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;
    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 1);

    oakLogCube = new OakLog(0.2f);      oakLogCube->Init();
    grassBlock = new GrassBlock(0.2f);  grassBlock->Init();
    stairs = new Stairs(0.2f);      stairs->Init();
    leaves = new Leaves(0.2f);      leaves->Init();
    glassPanel = new Panel(0.2f);       glassPanel->Init();
    door = new Door(0.5f);        door->Init();

    const char* ft[5] = {
        "flower_blue_orchid.png",
        "flower_dandelion.png",
        "flower_tulip_white.png",
        "flower_oxeye_daisy.png",
        "flower_rose.png"
    };
    for (int i = 0; i < 5; i++) {
        flowers[i] = new Flower(0.1f, ft[i]);
        flowers[i]->Init();
    }

    hill = new Hill(4.0f, 1.0f, 16, 3.0f);
    hill->Init();
    robot = new Robot(0, 0, 1.0f / 20.0f);
    projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f);
    GLuint depthShader = createProgram(depthVertexShaderSource, depthFragmentShaderSource);
    sceneShader = createProgram(sceneVertexShaderSource, sceneFragmentShaderSource);
    const unsigned SHW = 1024, SHH = 1024;
    GLuint depthFBO, depthMap;
    glGenFramebuffers(1, &depthFBO);
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHW, SHH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float bc[4] = { 1,1,1,1 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, bc);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE); glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glm::vec3 lightDir(0.4f, -1.0f, 0.4f);
    glm::vec3 lightPos(0.1f, 1.0f, 2.0f);
    glm::mat4 lV = glm::lookAt(lightPos, glm::vec3(0), glm::vec3(0, 1, 0));
    glm::mat4 lP = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 1.0f, 50.0f);
    glm::mat4 lightSpace = lP * lV;
    glm::vec3 hillPosition(0.0f, 0.08f, planeOffset + hillBaseRadius - 17.8f);
    glm::vec3 hillRotation(0.0f, -90.0f, 0.0f);
    while (!glfwWindowShouldClose(win)) {
        float now = (float)glfwGetTime();
        static float last = now;
        float dt = now - last;
        last = now;

        processInput(win, dt);
        updateRobot(dt, win);
        glViewport(0, 0, SHW, SHH);
        glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glUseProgram(depthShader);
        glUniformMatrix4fv(
            glGetUniformLocation(depthShader, "lightSpaceMatrix"),
            1, GL_FALSE, glm::value_ptr(lightSpace)
        );
        renderScene(
            glm::mat4(1.0f), glm::mat4(1.0f),
            lightSpace, lightDir, dirLightColor,
            camera.Position, depthMap
        );
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, 800, 600);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(sceneShader);
        glUniformMatrix4fv(
            glGetUniformLocation(sceneShader, "view"),
            1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix())
        );
        glUniformMatrix4fv(
            glGetUniformLocation(sceneShader, "projection"),
            1, GL_FALSE, glm::value_ptr(projection)
        );
        glUniformMatrix4fv(
            glGetUniformLocation(sceneShader, "lightSpaceMatrix"),
            1, GL_FALSE, glm::value_ptr(lightSpace)
        );
        glUniform3fv(
            glGetUniformLocation(sceneShader, "lightDir"),
            1, glm::value_ptr(lightDir)
        );
        glUniform3fv(
            glGetUniformLocation(sceneShader, "lightColor"),
            1, glm::value_ptr(dirLightColor)
        );
        glUniform3fv(
            glGetUniformLocation(sceneShader, "cornerLightPos"),
            1, glm::value_ptr(glm::vec3(0, 5, 0))
        );
        glUniform3fv(
            glGetUniformLocation(sceneShader, "cornerLightColor"),
            1, glm::value_ptr(cornerLightColor)
        );
        glUniform3fv(
            glGetUniformLocation(sceneShader, "viewPos"),
            1, glm::value_ptr(camera.Position)
        );
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glUniform1i(
            glGetUniformLocation(sceneShader, "shadowMap"),
            0
        );
        glm::mat4 hillModel = glm::translate(glm::mat4(2.5f), hillPosition);
        hillModel = glm::rotate(hillModel, glm::radians(hillRotation.x), glm::vec3(1, 0, 0));
        hillModel = glm::rotate(hillModel, glm::radians(hillRotation.y), glm::vec3(0, 1, 0));
        hillModel = glm::rotate(hillModel, glm::radians(hillRotation.z), glm::vec3(0, 0, 1));
        hill->Draw(camera.GetViewMatrix(), projection, hillModel, lightSpace, lightDir, dirLightColor, camera.Position, depthMap);
        renderScene(
            camera.GetViewMatrix(), projection,
            lightSpace, lightDir, dirLightColor,
            camera.Position, depthMap
        );

        robot->Position = robotPos;
        robot->Yaw = glm::radians(robotYaw);
            robot->Update(dt, win);
        robot->Draw(camera.GetViewMatrix(), projection);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    delete oakLogCube; delete grassBlock; delete stairs; delete leaves; delete glassPanel; delete door;
    for (auto& f : flowers) delete f;
    delete robot; delete hill;
    glfwTerminate();
    return 0;
}
