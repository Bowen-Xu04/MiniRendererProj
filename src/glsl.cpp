// 参考已有代码：https://learnopengl-cn.github.io/01%20Getting%20started/03%20Hello%20Window/
#include "glsl.hpp"
#include "utils.hpp"

float GLSL_Window::fullscreenRectangular_vertices[12] = {
    1.0f, 1.0f, 0.0f,   // 右上角
    1.0f, -1.0f, 0.0f,  // 右下角
    -1.0f, -1.0f, 0.0f, // 左下角
    -1.0f, 1.0f, 0.0f   // 左上角
};

unsigned int GLSL_Window::fullscreenRectangular_indices[6] = {
    0, 1, 2, // 第一个三角形
    0, 2, 3// 第二个三角形
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}