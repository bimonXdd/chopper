// ---------------------------- Includes -------------------------- //
#include <stdlib.h>         // C++ standard library
#include <stdexcept>
#include <stack>            // We use the standard C++ stack implementation to create model matrix stacks
#include <vector>
#include <thread>         // Threading
#include <stdio.h>          // Input/Output
#include <GL/glew.h>      // OpenGL Extension Wrangler -
#include <GLFW/glfw3.h>     // Windows and input
#include <glm/glm.hpp>      // OpenGL math library

#include "shader_util.h"
#include "texture_util.h"

// --------------- Forward declarations ------------- //
GLuint createQuad(float width);

size_t textureIndex = 0; //We put texture handles in a vector and specify the current index
size_t bumpTextureIndex = 0;
std::vector<GLuint> textureHandles = std::vector<GLuint>();
std::vector<GLuint> bumpTextureHandles = std::vector<GLuint>();

#define CONCAT_PATHS(A, B) A "/" B
#define ADD_ROOT(B) CONCAT_PATHS(TASK_ROOT_PATH, B)

shader_prog shader(ADD_ROOT("shaders/texture.vert.glsl"), ADD_ROOT("shaders/texture.frag.glsl"));
shader_prog defaultShader(ADD_ROOT("shaders/default.vert.glsl"), ADD_ROOT("shaders/default.frag.glsl"));

GLuint quadVAO;
GLuint lightVAO;

glm::vec3 lightPosition = glm::vec3(1.0f);
glm::vec3 lightPositionCam = glm::vec3(1.0f);

/**
 * Creates a quad
 */
GLuint createQuad(float width, shader_prog* shader) {
    GLuint vertexArrayHandle;
    GLuint arrayBufferHandle;

    glGenVertexArrays(1, &vertexArrayHandle);
    glBindVertexArray(vertexArrayHandle);
    glGenBuffers(1, &arrayBufferHandle);

    GLfloat vertexData[] = {
        //X     Y       Z       U     V       Normal
        -width, -width, 0.0f,   0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
         width, -width, 0.0f,   1.0f, 0.0f,   0.0f, 0.0f, 1.0f,
        -width,  width, 0.0f,   0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
         width, -width, 0.0f,   1.0f, 0.0f,   0.0f, 0.0f, 1.0f,
         width,  width, 0.0f,   1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        -width,  width, 0.0f,   0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
    };

    glBindBuffer(GL_ARRAY_BUFFER, arrayBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

    GLuint loc = glGetAttribLocation(shader->getProg(), "position");
    if (loc < 0) throw (std::runtime_error(std::string("Location not found in shader program for variable ") + "position"));
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (const GLvoid*)(0*sizeof(float)));

    loc = glGetAttribLocation(shader->getProg(), "uv");
    if (loc < 0) throw (std::runtime_error(std::string("Location not found in shader program for variable ") + "color"));
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (const GLvoid*)(3*sizeof(float)));

    loc = glGetAttribLocation(shader->getProg(), "normal");
    if (loc < 0) throw (std::runtime_error(std::string("Location not found in shader program for variable ") + "normal"));
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (const GLvoid*)(5*sizeof(float)));

    glBindVertexArray(0);
    return vertexArrayHandle;
}

/**
 * Draws the scene (quad and light)
 */
void drawScene() {
    float speed = 0.50f;
    std::stack<glm::mat4> ms;
    ms.push(glm::mat4(1.0));

    ms.push(ms.top()); //Textured quad
        /**
         * --Task--
         * Make the plane rotate in the range [-45, 45] degrees around the y axis
         * Use a cosine function for interpolation.
         */
        shader.uniformMatrix4fv("modelMatrix", ms.top());

        //Bind the first texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureHandles.at(textureIndex));

        //Bind the second texture
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, bumpTextureHandles.at(bumpTextureIndex));

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    ms.pop();

    ms.push(ms.top()); //Light Source
        ms.top() = glm::translate(ms.top(), lightPosition);
        defaultShader.activate();
        defaultShader.uniformMatrix4fv("modelMatrix", ms.top());
        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    ms.pop();
}

// ---------------------------- Input -------------------------- //
// This method is called when keyboard event happens.
// Sets GLFW window should close flag to true, when escape key is pressed.
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
        if (key == GLFW_KEY_RIGHT) {
            textureIndex = (textureIndex + 1) % textureHandles.size();
        }
        if (key == GLFW_KEY_LEFT) {
            textureIndex = (textureIndex - 1 + textureHandles.size()) % textureHandles.size();
        }
        if (key == GLFW_KEY_UP) {
            bumpTextureIndex = (bumpTextureIndex + 1) % bumpTextureHandles.size();
        }
        if (key == GLFW_KEY_DOWN) {
            bumpTextureIndex = (bumpTextureIndex - 1 + bumpTextureHandles.size()) % bumpTextureHandles.size();
        }
        printf("Texture: %zu, Bump: %zu\n", textureIndex, bumpTextureIndex);
    }
}

// ---------------------------- Main -------------------------- //
int main(int argc, char *argv[]) {
    GLFWwindow *win;
    if (!glfwInit()) {
        exit (EXIT_FAILURE);
    }

    float screenWidth = 800;
    float screenHeight = 450;
    win = glfwCreateWindow(static_cast<int>(screenWidth), static_cast<int>(screenHeight), "Bump Mapping", NULL, NULL);

    if (!win) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(win);
    glewExperimental = GL_TRUE;

    GLenum status = glewInit();
    if(status != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(status));
    }

    const GLubyte* renderer = glGetString (GL_RENDERER);
    const GLubyte* version = glGetString (GL_VERSION);
    printf ("Renderer: %s\n", renderer);
    printf ("OpenGL version supported %s\n", version);

    glfwSetKeyCallback(win, key_callback);

    glm::vec3 viewer = glm::vec3(0.0, 0.0, 2.0);
    glm::mat4 perspective = glm::perspective(glm::radians(60.0f), screenWidth / screenHeight, 0.1f, 100.f);
    glm::mat4 view = glm::lookAt(
        viewer, //Position
        glm::vec3(0.0, 0.0, 0.0),  //LookAt
        glm::vec3(0.0, 1.0, 0.0)   //Up
    );

    shader.use();
    shader.uniformMatrix4fv("projectionMatrix", perspective);
    shader.uniformMatrix4fv("viewMatrix", view);

    defaultShader.use();
    defaultShader.uniformMatrix4fv("projectionMatrix", perspective);
    defaultShader.uniformMatrix4fv("viewMatrix", view);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // -------------- Load textures ------------- //
    textureHandles.push_back(load_texture(GL_TEXTURE_2D, GL_RGB, ADD_ROOT("data/lines.png")));
    textureHandles.push_back(load_texture(GL_TEXTURE_2D, GL_RGBA, ADD_ROOT("data/grid.png")));
    textureHandles.push_back(load_texture(GL_TEXTURE_2D, GL_RGB, ADD_ROOT("data/ut256.png")));
    textureHandles.push_back(load_texture(GL_TEXTURE_2D, GL_RGB, ADD_ROOT("data/bumpTexture3.jpg")));

    bumpTextureHandles.push_back(load_texture(GL_TEXTURE_2D, GL_RGB, ADD_ROOT("data/bumpTexture.jpg")));
    bumpTextureHandles.push_back(load_texture(GL_TEXTURE_2D, GL_RGB, ADD_ROOT("data/bumpTexture.png")));
    bumpTextureHandles.push_back(load_texture(GL_TEXTURE_2D, GL_RGB, ADD_ROOT("data/bumpTexture2.jpg")));
    bumpTextureHandles.push_back(load_texture(GL_TEXTURE_2D, GL_RGB, ADD_ROOT("data/bumpTexture3.jpg")));

    shader.activate();
    GLint texLoc = glGetUniformLocation(shader, "texture");
    glUniform1i(texLoc, 0);
    texLoc = glGetUniformLocation(shader, "bumpTexture");
    glUniform1i(texLoc, 1);

    // -------------- Create objects ------------- //
    quadVAO = createQuad(1.0f, &shader);
    lightVAO = createQuad(0.02f, &defaultShader);

    while (!glfwWindowShouldClose(win)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

         //Move our light on a trajectory
        lightPosition.x = static_cast<float>(0.8 * sin(glfwGetTime()));
        lightPosition.y = static_cast<float>(0.5 * cos(glfwGetTime()));
        lightPosition.z = static_cast<float>(0.1 * sin(glfwGetTime()) + 0.8);
        lightPositionCam = glm::vec3(view * glm::vec4(lightPosition, 1.0));
        shader.activate();
        shader.uniform3f("lightPosition", lightPositionCam.x, lightPositionCam.y, lightPositionCam.z);

        drawScene();
        glfwSwapBuffers(win);

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        glfwPollEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
    return 0;
}
