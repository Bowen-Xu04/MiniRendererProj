// 参考已有代码
// https://learnopengl-cn.github.io/
// https://zhuanlan.zhihu.com/p/51387524
// https://zhuanlan.zhihu.com/p/11890542295
#ifndef GLSL_H
#define GLSL_H

#include <iostream>
#include <iomanip>
#include <cstring>
#include <chrono>
#include <type_traits>
#include "camera.hpp"
#include "shader.hpp"
#include "group.hpp"
#include "object3d.hpp"
#include "scene_parser.hpp"
#include "image.hpp"
#include "utils.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

class GLSL_Window {
private:
    static float fullscreenRectangular_vertices[12];
    static unsigned int fullscreenRectangular_indices[6];

    int width, height;
    std::string name;

    struct RendererProperty {
        int usingAS;
        int NEE;
        float RR;
        float invRR;
    };
    RendererProperty rendererProperty;

    int frameCount, max_frameCount;

    GLFWwindow* window;

    unsigned int VAO, VBO, EBO;

public:
    GLSL_Window(int _width, int _height, int _max_frameCount, bool _usingAS, bool _NEE, float _RR, std::string _name) :
        frameCount(0), max_frameCount(_max_frameCount > 0 ? _max_frameCount : INT32_MAX), VAO(0), VBO(0), EBO(0), width(_width), height(_height), name(_name) {
        rendererProperty.usingAS = (int)_usingAS;
        rendererProperty.NEE = (int)_NEE;
        rendererProperty.RR = _RR;
        rendererProperty.invRR = 1.f / _RR;

        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

        window = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);

        if (window == NULL)
        {
            std::cout << "ERROR: Failed to create GLFW window" << std::endl;
            glfwTerminate();
            exit(1);
        }

        glfwMakeContextCurrent(window);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "ERROR: Failed to initialize GLAD" << std::endl;
            exit(1);
        }

        glViewport(0, 0, width, height);
        //glfwSetWindowAttrib(window, GLFW_RESIZABLE, false);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        // 检查所需扩展支持
        const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
        if (!extensions) {
            std::cerr << "glGetString(GL_EXTENSIONS) failed!" << std::endl;
        }
        else {
            bool supportTexBuffer = strstr(extensions, "ARB_texture_buffer_object") != nullptr;
            bool supportRGB8UI = strstr(extensions, "EXT_texture_integer") != nullptr;

            std::cout << "ARB_texture_buffer_object supported: " << supportTexBuffer << std::endl;
            std::cout << "EXT_texture_integer supported: " << supportRGB8UI << std::endl;
        }

        // 检查 OpenGL 版本
        GLint major, minor;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        std::cout << "OpenGL version: " << major << "." << minor << std::endl;
    }

    ~GLSL_Window() {
        // 不销毁 window，因其指向不完整的类型
        // if (window != nullptr) {
        //     delete window;
        // }
    }

    // void initProgram() {
    // }
    // void bindVAO(const float* vertices) {
    //     glGenVertexArrays(1, &VAO);
    //     glBindVertexArray(VAO);
    // }
    // void bindVBO(const float* vertices) {
    //     glGenBuffers(1, &VBO);
    //     // 1. 复制顶点数组到缓冲中供OpenGL使用
    //     glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //     glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //     // 2. 设置顶点属性指针
    //     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    //     glEnableVertexAttribArray(0);
    // }
    // void bindEBO(const unsigned int* indices) {
    //     glGenBuffers(1, &EBO);
    //     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //     glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    // }
    // void use_shaderProgram() {
    //     glUseProgram(shaderProgram);
    // }

    template <typename T>
    unsigned int loadTextureBuffer(const std::vector<T>& data, int channels) {
        if (std::is_same<int, T>::value == 0 && std::is_same<float, T>::value == 0) {
            printf("ERROR: Unsupported type.\n");
            exit(1);
        }

        unsigned int tbo;
        glGenBuffers(1, &tbo);
        glBindBuffer(GL_TEXTURE_BUFFER, tbo);
        glBufferData(GL_TEXTURE_BUFFER, data.size() * sizeof(T), data.data(), GL_STATIC_DRAW);

        unsigned int tboTexture;
        glGenTextures(1, &tboTexture);

        unsigned int format;
        switch (channels) {
        case 1:
            format = (std::is_same<int, T>::value ? GL_R32I : GL_R32F);
            break;
        case 2:
            format = (std::is_same<int, T>::value ? GL_RG32I : GL_RG32F);
            break;
        case 3:
            format = (std::is_same<int, T>::value ? GL_RGB32I : GL_RGB32F);
            break;
        case 4:
            format = (std::is_same<int, T>::value ? GL_RGBA32I : GL_RGBA32F);
            break;
        default:
            printf("ERROR: Invalid channel number.\n");
            exit(1);
        }
        //printf("gen tex buffer: %d %d %u\n", data.size(), channels, format);

        glBindTexture(GL_TEXTURE_BUFFER, tboTexture);
        // if (channels != 3 && channels != 4) {
        //     printf("ERROR: Invalid channel number.\n");
        //     exit(1);
        // }
        // if (dataType != GL_FLOAT && dataType != GL_INT) {
        //     printf("ERROR: Invalid data type.\n");
        //     exit(1);
        // }
        // unsigned int innerFormat, inputFormat;
        // if (channels == 3 && dataType == GL_FLOAT) {
        //     innerFormat = GL_RGB32F; inputFormat = GL_RGB;
        // }
        // else if (channels == 3 && dataType == GL_INT) {
        //     innerFormat = GL_RGB32I; inputFormat = GL_RGB_INTEGER;
        // }
        // else if (channels == 4 && dataType == GL_FLOAT) {
        //     innerFormat = GL_RGBA32F; inputFormat = GL_RGBA;
        // }
        // else if (channels == 3 && dataType == GL_FLOAT) {
        //     innerFormat = GL_RGBA32I; inputFormat = GL_RGB_INTEGER;
        // }
        // int size = calculate_texture_size(data.size() / channels);
        // std::vector<float> padded_data(data);
        // padded_data.resize(size * size * channels);
        // glBindTexture(GL_TEXTURE_2D, textureID);
        // glTexImage2D(GL_TEXTURE_2D, 0, innerFormat, size, size, 0, inputFormat, dataType, padded_data.data());
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexBuffer(GL_TEXTURE_BUFFER, format, tbo);

        glBindBuffer(GL_TEXTURE_BUFFER, 0);
        glBindTexture(GL_TEXTURE_BUFFER, 0);

        return tboTexture;
    }

    unsigned int loadTexture(std::string path) {
        return 0;
    }

    void saveImg(Image& img) {
        unsigned char* colorBuffer = new unsigned char[width * height * 3];
        glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, colorBuffer);

        for (int j = 0;j < height;j++) {
            for (int i = 0;i < width;i++) {
                int idx = j * width + i;
                Vector3f col(colorBuffer[3 * idx] * 1.0 / 255, colorBuffer[3 * idx + 1] * 1.0 / 255, colorBuffer[3 * idx + 2] * 1.0 / 255);
                //gamma_correction(col);
                img.SetPixel(i, j, col);
            }
        }

        delete[] colorBuffer;
    }

    void test() { // 参考已有代码

        float vertices[] = {
            //     ---- 位置 ----       ---- 颜色 ----     - 纹理坐标 -
                 1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // 右上
                 1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // 右下
                -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // 左下
                -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // 左上
        };

        // std::vector<std::string> g_supportExtensions;
        // g_supportExtensions.clear();
        // Shader cs("shaders/tests/cshader.comp", GL_COMPUTE_SHADER);
        // // GLuint computeShaderID = glCreateShader(GL_COMPUTE_SHADER);
        // // const GLchar* computeShaderSource = ;
        // // glShaderSource(computeShaderID, 1, &computeShaderSource, NULL);
        // // glCompileShader(computeShaderID);
        // GLuint shaderProgramID = glCreateProgram();
        // glAttachShader(shaderProgramID, cs.get_id());
        // glLinkProgram(shaderProgramID);
        // glUseProgram(shaderProgramID);
        // GLuint textureID;
        // glBindImageTexture(0, textureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        // glDispatchCompute(32, 32, 1);
        // glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        // return;
        // GLint maxUniformBlockSize;
        // glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize);
        // std::cout
        //     << "Maximun number of Uniform Block Size : "
        //     << maxUniformBlockSize
        //     << std::endl;
        // return;
        // const GLubyte* name = glGetString(GL_VENDOR);            //返回负责当前OpenGL实现厂商的名字
        // const GLubyte* biaoshifu = glGetString(GL_RENDERER);    //返回一个渲染器标识符，通常是个硬件平台
        // const GLubyte* OpenGLVersion = glGetString(GL_VERSION);    //返回当前OpenGL实现的版本号
        // const GLubyte* glsl = glGetString(GL_SHADING_LANGUAGE_VERSION);//返回着色预压编译器版本号
        // //const GLubyte* gluVersion = gluGetString(GLU_VERSION);    //返回当前GLU工具库版本
        // printf("OpenGL实现厂商的名字：%s\n", name);
        // printf("渲染器标识符：%s\n", biaoshifu);
        // printf("OpenGL实现的版本号：%s\n", OpenGLVersion);
        // printf("OpenGL着色语言版本：%s\n", glsl);
        //printf("GLU工具库版本：%s\n", gluVersion);
        // int n;
        // glGetIntegerv(GL_NUM_EXTENSIONS, &n);
        // for (int i = 0; i < n; i++) {
        //     std::string extension = (char*)glGetStringi(GL_EXTENSIONS, i);
        //     //g_supportExtensions.push_back(extension);
        //     std::cout << "Exts : " << extension << std::endl;;
        // }
        // return;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        // 初始化
        Shader verts("shaders/tests/vshader.vert", GL_VERTEX_SHADER);
        //verts.compile();

        Shader frags("shaders/tests/fshader.frag", GL_FRAGMENT_SHADER);
        //frags.compile();

        unsigned int shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, verts.get_id());
        glAttachShader(shaderProgram, frags.get_id());
        glLinkProgram(shaderProgram);

        int success = 0;
        char infoLog[512] = {};
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cout << "ERROR: Failed to link shader program." << std::endl << infoLog << std::endl;
        }

        // 1. 生成并绑定VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        // 2. 把顶点数组复制到缓冲中供OpenGL使用
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // 3. 复制我们的索引数组到一个索引缓冲中，供OpenGL使用
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(fullscreenRectangular_indices), fullscreenRectangular_indices, GL_STATIC_DRAW);

        // 纹理相关
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // texture1
        int width1, height1, channels1;
        unsigned char* data1 = readImageFromFile("mesh/LearnOpenGL/container.jpg", width1, height1, channels1, true);
        unsigned int texture1;
        glGenTextures(1, &texture1);
        glBindTexture(GL_TEXTURE_2D, texture1);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width1, height1, 0, GL_RGB, GL_UNSIGNED_BYTE, data1);
        glGenerateMipmap(GL_TEXTURE_2D);

        delete[] data1;

        // texture2
        int width2, height2, channels2;
        unsigned char* data2 = readImageFromFile("mesh/LearnOpenGL/awesomeface.png", width2, height2, channels2, true);
        unsigned int texture2;
        glGenTextures(1, &texture2);
        glBindTexture(GL_TEXTURE_2D, texture2);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width2, height2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2);
        glGenerateMipmap(GL_TEXTURE_2D);

        delete[] data2;

        glUseProgram(shaderProgram);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture2"), 1);

        // 4. 设定顶点属性指针
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        //glTexParameteri(GL_TEXTURE_2D, GL)

        glDeleteShader(verts.get_id());
        glDeleteShader(frags.get_id());

        int last_frameCount = 0, time_last = 0;
        auto start = std::chrono::system_clock::now();
        auto tmp = std::chrono::system_clock::now();
        auto end = std::chrono::system_clock::now();

        while (!glfwWindowShouldClose(window))
        {
            processInput(window);

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture1);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texture2);

            glUseProgram(shaderProgram);
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            glfwSwapBuffers(window);
            glfwPollEvents();

            ++frameCount;
            end = std::chrono::system_clock::now();
            time_last = std::chrono::duration_cast<std::chrono::milliseconds>(end - tmp).count();

            if (time_last >= FPS_CALC_INTERNAL) {
                std::cout << std::setprecision(4) << "FPS: " << (frameCount - last_frameCount) * 1000.f / time_last << " "
                    << "(" << time_last * 1.f / (frameCount - last_frameCount) << "ms/frame)\r";

                tmp = std::chrono::system_clock::now();
                last_frameCount = frameCount;
            }
        }

        glfwTerminate();
    }

    void run(SceneParser& scene, Image& img) {
        // 初始化
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        ShaderProgram PTProgram("shaders/vertexShader.vert", "shaders/PathTracer.frag");
        ShaderProgram screenProgram("shaders/vertexShader.vert", "shaders/screenShader.frag");

        // 1. 生成并绑定VAO
        //glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        // 2. 把顶点数组复制到缓冲中供OpenGL使用
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(fullscreenRectangular_vertices), fullscreenRectangular_vertices, GL_STATIC_DRAW);
        // 3. 复制我们的索引数组到一个索引缓冲中，供OpenGL使用
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(fullscreenRectangular_indices), fullscreenRectangular_indices, GL_STATIC_DRAW);
        // 4. 设定顶点属性指针
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // 5. 生成场景数据纹理
        TriangleData triangleData;
        scene.getGroup()->generateTriangleData(triangleData);
        // triangleData.printInfo();

        // 5-0. BVH纹理和AABB纹理相关
        unsigned int BVHTexture = 0, AABBTexture = 0;
        if (rendererProperty.usingAS) {
            //Object3D::create_global_BVH();

            BVH global_bvh;
            global_bvh.build(&triangleData.triangles);

            int cnt = 0;
            global_bvh.generateBVHData(cnt, triangleData.bvh, triangleData.boxes);

            // for (int i = 0;i < triangleData.bvh.size() / 4;i++) {
            //     printf("#%d [%d,%d,%d,%d] (%f,%f,%f) (%f,%f,%f)\n",
            //         i, triangleData.bvh[4 * i], triangleData.bvh[4 * i + 1], triangleData.bvh[4 * i + 2], triangleData.bvh[4 * i + 3],
            //         triangleData.boxes[6 * i], triangleData.boxes[6 * i + 1], triangleData.boxes[6 * i + 2],
            //         triangleData.boxes[6 * i + 3], triangleData.boxes[6 * i + 4], triangleData.boxes[6 * i + 5]);
            // }
            // triangleData.vertices.resize(3 * Object3D::get_vertex_cnt());
            // for (int i = 0;i < Object3D::get_primitive_cnt();i++) {
            //     int i0, i1, i2;
            //     Object3D::get_object(i)->getVertexIndices(i0, i1, i2);
            //     triangleData.vertexIndices.push_back(i0);
            //     triangleData.vertexIndices.push_back(i1);
            //     triangleData.vertexIndices.push_back(i2);
            //     triangleData.vertexIndices.push_back(Object3D::get_object(i)->get_material()->get_id());
            // }
            // std::vector<int> temp;
            // temp.clear();
            // //temp.resize(triangleData.vertexIndices.size());
            // for (int i = 0;i < triangleData.triangles.size();i++) {
            //     int idx = triangleData.triangles[i]->get_unique_triangleID();
            //     temp.insert(temp.end(), triangleData.vertexIndices.begin() + 4 * idx, triangleData.vertexIndices.begin() + 4 * idx + 4);
            // }
            // triangleData.vertexIndices = temp;
            // printf("=============\n");
            // triangleData.printInfo();
            // int cnt = 0;
            // Object3D::.generateBVHData(bvh.root, cnt, triangleData.bvh, triangleData.boxes);
            //triangleData.bvh.resize(triangleData.vertexIndices.size());
            //triangleData.boxes.resize()

            BVHTexture = loadTextureBuffer<int>(triangleData.bvh, 4);
            AABBTexture = loadTextureBuffer<float>(triangleData.boxes, 3);
        }

        // for (int i = 0;i < triangleData.vertexIndices.size() / 4;i++) {
        //     if (Material::get_material(triangleData.vertexIndices[4 * i + 3])->hasEmission()) {
        //         triangleData.emissive_vertexIndices.insert(
        //             triangleData.emissive_vertexIndices.end(),
        //             triangleData.vertexIndices.begin() + 4 * i,
        //             triangleData.vertexIndices.begin() + 4 * i + 3
        //         );
        //         triangleData.emissive_vertexIndices.push_back(i);
        //     }
        // }
        // triangleData.vertices.resize(3 * Object3D::get_vertex_cnt());
        // for (int i = 0;i < Object3D::get_primitive_cnt();i++) {
        //     Object3D::get_object(i)->appendTriangleData(triangleData);
        // }
        //scene.getGroup()->generateTriangleData(triangleData);

        for (auto triangle : triangleData.triangles) {
            int i0, i1, i2;
            triangle->getVertexIndices(i0, i1, i2);

            //printf("[%d,%d,%d] (%d)\n", i0, i1, i2, triangle->get_material() != nullptr);

            triangleData.vertexIndices.push_back(i0);
            triangleData.vertexIndices.push_back(i1);
            triangleData.vertexIndices.push_back(i2);
            triangleData.vertexIndices.push_back(triangle->get_material()->get_id());

            if (triangle->get_material()->hasEmission()) {
                triangleData.emissive_vertexIndices.insert(
                    triangleData.emissive_vertexIndices.end(), triangleData.vertexIndices.end() - 4, triangleData.vertexIndices.end()
                );
            }
        }

        triangleData.printInfo();
        // printf("========materials\n");
        // for (int i = 0;i < Material::get_material_cnt();i++) {
        //     printf("#%d D:", i);
        //     printvec3(Material::get_material(i)->getDiffuseColor());
        //     printf("E:");
        //     printvec3(Material::get_material(i)->getEmission());
        //     printf("\n");
        // }

        // 5-1. 生成顶点索引纹理（起到VBO的作用）
        unsigned int vertexIndicesTBOTexture = loadTextureBuffer<int>(triangleData.vertexIndices, 4);
        // 5-2. 生成三角形光源的顶点索引纹理
        unsigned emissive_vertexIndicesTBOTexture = loadTextureBuffer<int>(triangleData.emissive_vertexIndices, 4);
        // 5-3. 生成顶点坐标纹理
        unsigned int verticesTBOTexture = loadTextureBuffer<float>(triangleData.vertices, 3);
        // // 5-4. 生成BVH纹理
        // unsigned int  = loadTextureBuffer<int>(triangleData.bvh, 4);
        // // 5-5. 生成AABB纹理
        // unsigned int AABBTexture = loadTextureBuffer<float>(triangleData.boxes, 3);

        //PTProgram.set("")

        // 6. 若使用NVIDIA独立显卡，则创建颜色纹理缓冲；若使用集成显卡，则创建帧缓冲并附加颜色纹理
#ifdef USE_NVIDIA_GPU
#ifdef NVIDIA_TEST        
        unsigned int bufferSize = width * height * 4 * sizeof(unsigned char);
        // 创建双PBO
        GLuint pbo[2];
        glGenBuffers(2, pbo);
        for (int i = 0; i < 2; i++) {
            glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[i]);
            glBufferData(GL_PIXEL_PACK_BUFFER, bufferSize, NULL, GL_STREAM_READ);
        }

        // 创建双TBO
        GLuint tbo[2], tboTexture[2];
        glGenBuffers(2, tbo);
        glGenTextures(2, tboTexture);
        for (int i = 0; i < 2; i++) {
            glBindBuffer(GL_TEXTURE_BUFFER, tbo[i]);
            glBufferData(GL_TEXTURE_BUFFER, bufferSize, NULL, GL_DYNAMIC_DRAW);

            glBindTexture(GL_TEXTURE_BUFFER, tboTexture[i]);
            glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8, tbo[i]);
            int eno = glGetError();
            printf("[%d]\n", eno);
        }

        // 状态变量
        int currentPBO = 0;
        int currentTBO = 0;
        GLsync pboFence = nullptr;
#endif
        unsigned char* lastFrameColor = new unsigned char[width * height * 3];
        for (int i = 0;i < width * height * 3;i++) {
            lastFrameColor[i] = 0;
        }
        // glReadPixels(0, 0, img.Width(), img.Height(), GL_RGB, GL_UNSIGNED_BYTE, colorBuffer);
        // for (int j = 0;j < img.Height();j++) {
        //     for (int i = 0;i < img.Width();i++) {
        //         int idx = j * img.Width() + i;
        //         Vector3f col(colorBuffer[3 * idx] * 1.0 / 255, colorBuffer[3 * idx + 1] * 1.0 / 255, colorBuffer[3 * idx + 2] * 1.0 / 255);
        //         //gamma_correction(col);
        //         img.SetPixel(i, j, col);
        //     }
        // }
        //delete[] colorBuffer;

        unsigned int lastFrameColorTBO;
        glGenBuffers(1, &lastFrameColorTBO);
        //glBindBuffer(GL_TEXTURE_BUFFER, lastFrameColorTBO);
        //glBufferData(GL_TEXTURE_BUFFER, width * height * 3 * sizeof(unsigned char), lastFrameColor, GL_STATIC_DRAW);

        unsigned int lastFrameColorTBOTexture;
        glGenTextures(1, &lastFrameColorTBOTexture);
        // glBindTexture(GL_TEXTURE_BUFFER, lastFrameColorTBOTexture);
        // glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB8UI, lastFrameColorTBO);

        // glBindBuffer(GL_TEXTURE_BUFFER, lastFrameColorTBO);
        // glBufferData(GL_TEXTURE_BUFFER, data.size() * sizeof(T), data.data(), GL_STATIC_DRAW);

#else
        PTProgram.use();

        unsigned int fbo;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        unsigned int lastFrameColorBuffer;
        glGenTextures(1, &lastFrameColorBuffer);
        glBindTexture(GL_TEXTURE_2D, lastFrameColorBuffer);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        //glBindTexture(GL_TEXTURE_2D, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lastFrameColorBuffer, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            printf("ERROR: Framebuffer is not complete.\n");
            exit(1);
        }
#endif

        //glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //glTexParameteri(GL_TEXTURE_2D, GL)

        Matrix3f rm = scene.getCamera()->get_rotation_matrix();
        GLfloat rotation_matrix[9] = {
            rm(0, 0), rm(0, 1), rm(0, 2),
            rm(1, 0), rm(1, 1), rm(1, 2),
            rm(2, 0), rm(2, 1), rm(2, 2)
        };

        // for (int i = 0;i < 9;i++) {
        //     printf("%f ", rotation_atrix[i]);
        //     if (i % 3 == 2) {
        //         printf("\n");
        //     }
        // }

        if (Material::get_material_cnt() > MAX_MATERIAL_CNT) {
            printf("ERROR: Too many materials.\n");
            exit(1);
        }

        PTProgram.use();
        //PTProgram.set("time", (float)(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() * (1e-6)));
        //PTProgram.set("frameCnt", frameCount);
#ifdef USE_NVIDIA_GPU
        PTProgram.set("USE_NVIDIA_GPU", 1);
#else
        PTProgram.set("USE_NVIDIA_GPU", 0);
#endif
        PTProgram.set("RR", rendererProperty.RR);
        PTProgram.set("invRR", rendererProperty.invRR);
        //printf("%f %f\n", rendererProperty.RR, rendererProperty.invRR);
        PTProgram.set("usingAS", rendererProperty.usingAS);
        PTProgram.set("NEE", rendererProperty.NEE);
        PTProgram.set("background_color", scene.getBackgroundColor());
        PTProgram.set("total_light_area", scene.get_light_area());
        PTProgram.set("primitiveCnt", (int)(triangleData.vertexIndices.size() / 4));
        PTProgram.set("emissive_primitiveCnt", (int)(triangleData.emissive_vertexIndices.size() / 4));

        // printf("%f %f %d %d\n", rendererProperty.RR, rendererProperty.invRR, rendererProperty.usingAS, rendererProperty.NEE);
        // printvec3(scene.getBackgroundColor()); printvec3(scene.get_light_area());
        // printf("%d %d\n", triangleData.vertexIndices.size() / 4, triangleData.emissive_vertexIndices.size() / 4);

        PTProgram.set("camera.center", scene.getCamera()->get_center());
        // PTProgram.set("camera.direction", scene.getCamera()->get_direction());
        // PTProgram.set("camera.up", scene.getCamera()->get_up());
        PTProgram.set("camera.width", width);
        PTProgram.set("camera.height", height);
        //PTProgram.set("camera.angle", scene.getCamera()->get_perspectiveData().x());
        PTProgram.set("camera.invfx", scene.getCamera()->get_perspectiveData().y());
        PTProgram.set("camera.invfy", scene.getCamera()->get_perspectiveData().z());
        PTProgram.set("camera.rotation_matrix", rotation_matrix);

        for (int i = 0;i < Material::get_material_cnt();++i) {
            const std::string pref = "materials[" + std::to_string(i) + "]";
            PTProgram.set((pref + ".type").c_str(), (int)(Material::get_material(i)->get_type()));
            PTProgram.set((pref + ".diffuse").c_str(), Material::get_material(i)->getDiffuseColor());
            PTProgram.set((pref + ".emission").c_str(), Material::get_material(i)->getEmission());
            PTProgram.set((pref + ".diffuse_texture_ID").c_str(), -1);
            // printf("#%d %s=", i, (pref + ".diffuse").c_str());
            // printvec3(Material::get_material(i)->getDiffuseColor());
            // printf("%s=", (pref + ".emission").c_str());
            // printvec3(Material::get_material(i)->getEmission());
            // printf("\n");
        }

        // int location = glGetUniformLocation(PTProgram.get_id(), "camera.rotation_matrix");
        // //glUseProgram(programID);
        // // const GLfloat mat[9] = {
        // //     value[0][0],value[0][1]
        // // }
        // glUniformMatrix3fv(location, 1, GL_FALSE, rotation_matrix);

        int last_frameCount = 0, time_last = 0;
        auto start = std::chrono::system_clock::now();
        auto tmp = std::chrono::system_clock::now();
        auto end = std::chrono::system_clock::now();

        int eno = 0;

        while (!glfwWindowShouldClose(window))
        {
            processInput(window);

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // unsigned char* lastFrameColor = new unsigned char[width * height * 3];
            // for (int i = 0;i < width * height * 3;i++) {
            //     lastFrameColor[i] = 0;
            // }

            // unsigned int lastFrameColorTBO;
            // glGenBuffers(1, &lastFrameColorTBO);
            // glBindBuffer(GL_TEXTURE_BUFFER, lastFrameColorTBO);
            // glBufferData(GL_TEXTURE_BUFFER, width * height * 3 * sizeof(unsigned char), lastFrameColor, GL_STATIC_DRAW);

            // // unsigned int lastFrameColorTBOTexture;
            // // glGenTextures(1, &lastFrameColorTBOTexture);
            // glBindTexture(GL_TEXTURE_BUFFER, lastFrameColorTBOTexture);
            // glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB8UI, lastFrameColorTBO);

            // glBindBuffer(GL_TEXTURE_BUFFER, lastFrameColorTBO);
            // glBufferData(GL_TEXTURE_BUFFER, width * height * 3 * sizeof(unsigned char), lastFrameColor, GL_STATIC_DRAW);

            // glBindTexture(GL_TEXTURE_BUFFER, lastFrameColorTBOTexture);
            // glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB8UI, lastFrameColorTBO);

            // Pass 1：使用PTProgram对场景进行路径追踪，并将结果输出到缓冲区
            PTProgram.use();
            //PTProgram.set("time", (float)(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() * (1e-6)));
            PTProgram.set("frameCnt", frameCount);

            PTProgram.set("vertexIndices", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_BUFFER, vertexIndicesTBOTexture);
            PTProgram.set("emissive_vertexIndices", 1);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_BUFFER, emissive_vertexIndicesTBOTexture);
            PTProgram.set("vertices", 2);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_BUFFER, verticesTBOTexture);

            if (rendererProperty.usingAS) {
                PTProgram.set("BVH", 3);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_BUFFER, BVHTexture);
                PTProgram.set("AABB", 4);
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_BUFFER, AABBTexture);
            }

#ifdef USE_NVIDIA_GPU



            PTProgram.set("lastFrameColorBuffer", 5);
            glActiveTexture(GL_TEXTURE5);
            //glBindTexture(GL_TEXTURE_2D, lastFrameColorTBOTexture);

            glBindBuffer(GL_TEXTURE_BUFFER, lastFrameColorTBO);
            glBufferData(GL_TEXTURE_BUFFER, width * height * 3 * sizeof(unsigned char), lastFrameColor, GL_STATIC_DRAW);

            glBindTexture(GL_TEXTURE_2D, lastFrameColorTBOTexture);
            glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB8UI, lastFrameColorTBO);

#ifdef NVIDIA_TEST
            PTProgram.set("lastFrameColorTBO", 5);
            glBindTexture(GL_TEXTURE_BUFFER, tboTexture[currentTBO]);
#endif
            //glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB8I, lastFrameColorTBO);

            // glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            // glBindTexture(GL_TEXTURE_2D, lastFrameColorBuffer);            
#else
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glBindTexture(GL_TEXTURE_2D, lastFrameColorBuffer);
#endif

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            //eno = glGetError();
            //printf("1 %d\n", eno);
            //glDrawArrays(GL_TRIANGLES, 0, 6);
#ifdef USE_NVIDIA_GPU
            glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, lastFrameColor);

#ifdef NVIDIA_TEST
            // ===== 3. 确保前一帧PBO操作完成 =====
            if (pboFence) {
                GLenum waitStatus = glClientWaitSync(pboFence, 0, 1000000000); // 1秒超时
                if (waitStatus == GL_ALREADY_SIGNALED || waitStatus == GL_CONDITION_SATISFIED) {
                    glDeleteSync(pboFence);
                    pboFence = nullptr;
                }
            }
            eno = glGetError();
            //printf("2 %d\n", eno);
            // ===== 4. 异步读取当前帧到PBO =====
            int nextPBO = (currentPBO + 1) % 2;
            glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[nextPBO]);
            glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, 0);
            eno = glGetError();
            //printf("3 %d\n", eno);
            // ===== 5. 处理已完成的PBO数据 =====
            glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[currentPBO]);
            eno = glGetError();
            //printf("4 %d\n", eno);
            // 方法1：直接GPU-GPU复制 (最快)
            if (glCopyBufferSubData) { // 检查扩展支持
                int nextTBO = (currentTBO + 1) % 2;
                glBindBuffer(GL_COPY_READ_BUFFER, pbo[currentPBO]);
                glBindBuffer(GL_COPY_WRITE_BUFFER, tbo[nextTBO]);
                glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, bufferSize);
            }
            eno = glGetError();
            //printf("5 %d\n", eno);
            pboFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

            currentPBO ^= 1;
            currentTBO ^= 1;
            //glFinish();
#endif
            // glDeleteBuffers(1, &lastFrameColorTBO);
            // glDeleteTextures(1, &lastFrameColorTBOTexture);
            //printf("(%u %u %u)\n", lastFrameColor[(65536 + 370) * 3], lastFrameColor[(65536 + 370) * 3 + 1], lastFrameColor[(65536 + 370) * 3 + 2]);
            //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, lastFrameColor);
            // glBindBuffer(GL_READ_FRAMEBUFFER, fbo);
            // glBindBuffer(GL_DRAW_FRAMEBUFFER, 0);
            // glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
#else
            glBindVertexArray(0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            //Pass 2：使用screenProgram混合当前帧和先前帧，并将最终图像输出到屏幕
            screenProgram.use();
            screenProgram.set("frameCnt", frameCount);
            glBindVertexArray(VAO);
            glBindTexture(GL_TEXTURE_2D, lastFrameColorBuffer);
            //glDrawArrays(GL_TRIANGLES, 0, 6);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
#endif
            glfwSwapBuffers(window);
            glfwPollEvents();

#ifdef NVIDIA_TEST
            glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
            eno = glGetError();
            //printf("6 %d\n", eno);
#endif
            ++frameCount;
            end = std::chrono::system_clock::now();
            time_last = std::chrono::duration_cast<std::chrono::milliseconds>(end - tmp).count();

            //if (time_last >= FPS_CALC_INTERNAL || frameCount >= max_frameCount) {
            std::cout << std::setprecision(4) << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 1.0 / 1000 << "s"
                << "    Frame count: " << frameCount << "    FPS: " << 1000.0 / time_last
                << " (" << time_last << "ms/frame)\r";

            tmp = std::chrono::system_clock::now();
            //last_frameCount = frameCount;
        //}

            if (frameCount >= max_frameCount) {
                break;
            }
        }

        printf("\n");
        saveImg(img);

        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);

#ifdef USE_NVIDIA_GPU
        // glDeleteBuffers(2, lastFrameColorPBO);
        // glDeleteBuffers(2, lastFrameColorTBO);
        // glDeleteTextures(2, lastFrameColorTBOTexture);
        //delete[] lastFrameColor;
#else
        glDeleteFramebuffers(1, &lastFrameColorBuffer);
#endif

        glfwTerminate();

    }
};

#endif