// 独立实现，除了parse_argument函数参考了https://github.com/tanakh/cmdline
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
#include <string>

#include "scene_parser.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "group.hpp"
#include "light.hpp"
#include "renderer.hpp"
#include "sampler.hpp"
#include "shader.hpp"

//using namespace std;

std::string inputFile, outputFile;
Renderer* renderer;
int spp = 1, MSAA = 1;
float RR = 0.8;
bool NEE = false, AS = false;

enum RENDERING_MODE {
    //DEFAULT,
    WHITTED,
    //PATH_TRACING,
    PATH_TRACING_CPU,
    PATH_TRACING_GPU
};

RENDERING_MODE mode = RENDERING_MODE::WHITTED;

enum DEVICE {
    CPU,
    GPU
};

DEVICE device;

Sampler2D::SAMPLER2D_TYPE sampler2d_type = Sampler2D::SAMPLER2D_TYPE::BILINEAR_SAMPLER;

void parse_argument(int argc, char* argv[]) {
    printf("Parsing arguments...\n");
    bool path_tracing = false;

    // 参考已有代码：https://github.com/tanakh/cmdline
#ifdef _WIN32
    cmdline::parser parser;
    parser.add<std::string>("input", 'i', "input file", true, "");
    parser.add<std::string>("output", 'o', "output file", true, "");
    parser.add<std::string>("rm", 'r', "rendering method", true, "", cmdline::oneof<std::string>("Whitted", "RayTracing"));
    parser.add<std::string>("device", 'd', "rendering device", false, "CPU", cmdline::oneof<std::string>("CPU", "GPU"));
    parser.add("as", 'a', "acceleration structure");
    parser.add<int>("spp", 's', "samples per pixel", false, 1);
    parser.add<int>("msaa", 'm', "multi-sampling antialiasing", false, 1);
    parser.add("nee", 'n', "next event estimation");
    parser.add<std::string>("sampler", 't', "sampler of textures", false, "Bilinear", cmdline::oneof<std::string>("Bilinear", "Bicubic"));
    parser.add<float>("rr", 'p', "Russian roulette", false, 0.8);

    parser.parse_check(argc, argv);

    inputFile = parser.get<std::string>("input");
    std::cout << "input: " << inputFile << std::endl;

    outputFile = parser.get<std::string>("output");
    std::cout << "output: " << outputFile << std::endl;

    if (parser.get<std::string>("rm") == "Whitted") {
        mode = RENDERING_MODE::WHITTED;
    }
    else {
        path_tracing = true;
    }
    std::cout << "rm: " << parser.get<std::string>("rm") << std::endl;

    if (parser.exist("device")) {
        if (parser.get<std::string>("device") == "CPU" && path_tracing) {
            mode = RENDERING_MODE::PATH_TRACING_CPU;
        }
        else {
            if (!path_tracing) {
                printf("ERROR: Argument <rd> is not available for Whitted-style rendering mode. The default rendering device of Whitted-style raytracing is CPU.\n");
                exit(1);
            }
            mode = RENDERING_MODE::PATH_TRACING_GPU;
        }
    }
    std::cout << "device: " << parser.get<std::string>("device") << std::endl;

    if (parser.exist("as")) {
        AS = true;
    }
    std::cout << "as: " << AS << std::endl;

    if (parser.exist("spp")) {
        if (!path_tracing) {
            printf("ERROR: Argument <spp> is not available for Whitted-style rendering mode.\n");
            exit(1);
        }
        spp = parser.get<int>("spp");
        if (spp <= 0 && (!(spp == -1 && mode == RENDERING_MODE::PATH_TRACING_GPU))) { // 当使用OpenGL渲染时，spp=-1表示无限制地渲染下去，否则将在渲染到指定帧时停止渲染并保存图片
            printf("ERROR: Invalid spp.\n");
            exit(1);
        }
    }
    if (path_tracing) {
        std::cout << "spp: " << spp << std::endl;
    }

    if (parser.exist("msaa")) {
        MSAA = parser.get<int>("msaa");
        if (MSAA <= 0 || std::floor(std::sqrt(MSAA)) < std::sqrt(MSAA)) {
            printf("ERROR: Invalid MSAA multiple.\n");
            exit(1);
        }
    }
    std::cout << "msaa: " << MSAA << std::endl;

    if (parser.exist("nee")) {
        if (!path_tracing) {
            printf("ERROR: Argument <nee> is not available for this rendering mode.\n");
            exit(1);
        }
        NEE = true;
    }
    if (path_tracing) {
        std::cout << "as: " << NEE << std::endl;
    }

    if (parser.exist("sampler")) {
        if (parser.get<std::string>("sampler") == "Bilinear") {
            sampler2d_type = Sampler2D::SAMPLER2D_TYPE::BILINEAR_SAMPLER;
        }
        else {
            sampler2d_type = Sampler2D::SAMPLER2D_TYPE::BICUBIC_SAMPLER;
        }
    }
    std::cout << "sampler: " << parser.get<std::string>("sampler") << std::endl;

    if (parser.exist("rr")) {
        if (!path_tracing) {
            printf("ERROR: Argument <rr> is not available for Whitted-style rendering mode.\n");
            exit(1);
        }
        RR = parser.get<float>("rr");
        if (RR <= 0.0 || RR > 1.0) {
            printf("ERROR: Invalid RR value.\n");
            exit(1);
        }
        if (RR == 1.0) {
            printf("WARNING: The value of RR is 1.0, which may result in endless loops.\n");
        }
    }
    if (path_tracing) {
        std::cout << "rr: " << RR << std::endl;
    }

#else
    const char* optstring = "i:o:r:d::a::s::m::n::t::p::";
    const char* usage =
        "./build/RTProject --input: --output: --rm:\"Whitted\"/\"RayTracing\" <--device:[\"CPU\"]/\"GPU\"> <--as> <--spp:[1]> <--msaa:[1]> <--nee> <--sampler:[\"Bilinear\"]/\"Bicubic\"> <--rr:[0.8]>";

    // const int no_argument = 0;
    // const int required_argument = 1;
    // const int optional_argument = 2;
    int o, option_index;

    static struct option options[] = {
        {"input", required_argument, NULL, 'i'},
        {"output", required_argument, NULL, 'o'},
        {"rm", required_argument, NULL, 'r'},
        {"device", optional_argument, NULL, 'd'},
        {"as", optional_argument, NULL, 'a'},
        {"spp", optional_argument, NULL, 's'},
        {"msaa", optional_argument, NULL, 'm'},
        {"nee", optional_argument, NULL, 'n'},
        {"sampler", optional_argument, NULL, 't'},
        {"rr", optional_argument, NULL, 'p'}
    };
    while ((o = getopt_long(argc, argv, optstring, options, &option_index)) != -1) {
        printf("%s: %s\n", options[option_index].name, optarg);
        switch (o) {
        case 'i':
            inputFile = (std::string)(optarg);
            //printf("Input file: %s\n", optarg);
            break;
        case 'o':
            outputFile = (std::string)(optarg);
            //printf("Output file: %s\n", optarg);
            break;
        case 'r':
            if (strcmp(optarg, "Whitted") == 0) {
                mode = RENDERING_MODE::WHITTED;
            }
            else if (strcmp(optarg, "RayTracing") == 0) {
                path_tracing = true;
            }
            else {
                printf("ERROR: Unknown rendering mode.\n");
                exit(1);
            }
            //printf("Rendering mode: %s\n", optarg);
            break;
        case 'd':
            if (strcmp(optarg, "CPU") == 0 && path_tracing) {
                mode = RENDERING_MODE::PATH_TRACING_CPU;
            }
            else if (strcmp(optarg, "GPU") == 0) {
                if (!path_tracing) {
                    printf("ERROR: Argument <rd> is not available for Whitted-style rendering mode. The default rendering device of Whitted-style raytracing is CPU.\n");
                    exit(1);
                }
                mode = RENDERING_MODE::PATH_TRACING_GPU;
            }
            else {
                printf("ERROR: Unknown rendering device.\n");
                exit(1);
            }
            //printf("Rendering device: %s\n", optarg);
            break;
        case 'a':
            AS = true;
            break;
        case 's':
            if (!path_tracing) {
                printf("ERROR: Argument <spp> is not available for Whitted-style rendering mode.\n");
                exit(1);
            }
            if (sscanf(optarg, "%d", &spp) == EOF || (spp <= 0 && (!(spp == -1 && mode == RENDERING_MODE::PATH_TRACING_GPU)))) {
                printf("ERROR: Invalid spp.\n");
                exit(1);
            }
            //printf("spp: %s\n", optarg);
            break;
        case 'm':
            if (sscanf(optarg, "%d", &MSAA) == EOF || MSAA <= 0 || std::floor(std::sqrt(MSAA)) < std::sqrt(MSAA)) {
                printf("ERROR: Invalid MSAA multiple.\n");
                exit(1);
            }
            //printf("MSAA: %s\n", optarg);
            break;
        case 'n':
            if (!path_tracing) {
                printf("ERROR: Argument <NEE> is not available for Whitted-style rendering mode.\n");
                exit(1);
            }
            NEE = true;
            break;
        case 't':
            if (strcmp(optarg, "Bilinear") == 0) {
                sampler2d_type = Sampler2D::SAMPLER2D_TYPE::BILINEAR_SAMPLER;
            }
            else if (strcmp(optarg, "Bicubic") == 0) {
                sampler2d_type = Sampler2D::SAMPLER2D_TYPE::BICUBIC_SAMPLER;
            }
            else {
                printf("ERROR: Unknown sampler type.\n");
                exit(1);
            }
            break;
        case 'p':
            if (!path_tracing) {
                printf("ERROR: Argument <rr> is not available for Whitted-style rendering mode.\n");
                exit(1);
            }
            if (sscanf(optarg, "%f", &RR) == EOF || RR <= 0.0 || RR > 1.0) {
                printf("ERROR: Invalid RR value.\n");
                exit(1);
            }
            if (RR == 1.0) {
                printf("WARNING: The value of RR is 1.0, which may result in endless loops.\n");
            }
            break;
        case '?':
            printf("%s\n", usage);
            exit(1);
            break;
        }
    }
#endif

    // assert(argc == 5 || argc == 7);
    // for (int argNum = 1; argNum < argc; ++argNum) {
    //     std::cout << "Argument " << argNum << " is: " << argv[argNum] << std::endl;
    // }
    // inputFile = argv[1];
    // outputFile = argv[2];  // only bmp is allowed.   
    // if (std::string(argv[3]) == "Whitted") {
    //     assert(argc == 5);
    //     mode = RENDERING_MODE::WHITTED;
    // }
    // else if (std::string(argv[3]) == "RayTracing") {
    //     assert(argc == 7);
    //     if (std::string(argv[4]) == "CPU") {
    //         mode = RENDERING_MODE::PATH_TRACING_CPU;
    //         device = DEVICE::CPU;
    //     }
    //     else if (std::string(argv[4]) == "GPU") {
    //         mode = RENDERING_MODE::PATH_TRACING_GPU;
    //         device = DEVICE::GPU;
    //     }
    //     else {
    //         std::cout << "Unknown device." << std::endl;
    //         exit(1);
    //     }
    //     if (sscanf(argv[5], "%d", &spp) == EOF) {
    //         std::cout << "Invalid spp." << std::endl;
    //         exit(1);
    //     }
    // }
    // else {
    //     std::cout << "Unknown rendering mode." << std::endl;
    //     exit(1);
    // }
}

int main(int argc, char* argv[]) {

    // glfwInit();
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    // if (window == NULL)
    // {
    //     std::cout << "Failed to create GLFW window" << std::endl;
    //     glfwTerminate();
    //     return -1;
    // }
    // glfwMakeContextCurrent(window);
    // if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    // {
    //     std::cout << "Failed to initialize GLAD" << std::endl;
    //     return -1;
    // }
    // float vertices[] = {
    //     0.5f, 0.5f, 0.0f,   // 右上角
    //     0.5f, -0.5f, 0.0f,  // 右下角
    //     -0.5f, -0.5f, 0.0f, // 左下角
    //     -0.5f, 0.5f, 0.0f   // 左上角
    // };
    // unsigned int indices[] = {
    //     // 注意索引从0开始! 
    //     // 此例的索引(0,1,2,3)就是顶点数组vertices的下标，
    //     // 这样可以由下标代表顶点组合成矩形
    //     0, 1, 3, // 第一个三角形
    //     1, 2, 3  // 第二个三角形
    // };
    // const char* vertexShaderSource = "#version 330 core\n"
    //     "layout (location = 0) in vec3 aPos;\n"
    //     "void main()\n"
    //     "{\n"
    //     "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    //     "}\0";
    // const char* fragmentShaderSource = "#version 330 core\n"
    //     "out vec4 FragColor;\n"
    //     "void main()\n"
    //     "{\n"
    //     "    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    //     "}\0";
    // unsigned int VBO;
    // glGenBuffers(1, &VBO);
    // unsigned int EBO;
    // glGenBuffers(1, &EBO);
    // // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // //Shader verts("shaders/vshader.vert");
    // Shader verts("shaders/vshader.vert", GL_VERTEX_SHADER);
    // verts.compile();
    // Shader frags("shaders/fshader.frag", GL_FRAGMENT_SHADER);
    // frags.compile();
    // // unsigned int vertexShader;
    // // vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // // glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    // // glCompileShader(vertexShader);
    // // int  success;
    // // char infoLog[512];
    // // glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    // // if (!success)
    // // {
    // //     glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    // //     std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    // // }
    // // unsigned int fragmentShader;
    // // fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    // // glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    // // glCompileShader(fragmentShader);
    // unsigned int shaderProgram;
    // shaderProgram = glCreateProgram();
    // glAttachShader(shaderProgram, verts.get_id());
    // glAttachShader(shaderProgram, frags.get_id());
    // glLinkProgram(shaderProgram);
    // int success = 0;
    // char infoLog[512] = {};
    // glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    // if (!success) {
    //     glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    //     std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    // }
    // unsigned int VAO;
    // glGenVertexArrays(1, &VAO);
    // // 1. 绑定VAO
    // glBindVertexArray(VAO);
    // // 2. 把顶点数组复制到缓冲中供OpenGL使用
    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    // // 3. 设置顶点属性指针
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // glEnableVertexAttribArray(0);
    // glUseProgram(shaderProgram);
    // glBindVertexArray(VAO);
    // glDeleteShader(verts.get_id());
    // glDeleteShader(frags.get_id());
    // printf("[%d]\n", sizeof(indices) / sizeof(unsigned int));
    // while (!glfwWindowShouldClose(window))
    // {
    //     processInput(window);
    //     glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    //     glClear(GL_COLOR_BUFFER_BIT);
    //     glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    //     glfwSwapBuffers(window);
    //     glfwPollEvents();
    // }
    // glfwTerminate();
    // return 0;
    // const GLubyte* name = glGetString(GL_VENDOR); //返回负责当前OpenGL实现厂商的名字
    // const GLubyte* biaoshifu = glGetString(GL_RENDERER); //返回一个渲染器标识符，通常是个硬件平台
    // const GLubyte* OpenGLVersion = glGetString(GL_VERSION); //返回当前OpenGL实现的版本号
    // //const GLubyte* gluVersion = gluGetString(GLU_VERSION); //返回当前GLU工具库版本
    // printf("OpenGL实现厂商的名字：%s\n", name);
    // printf("渲染器标识符：%s\n", biaoshifu);
    // printf("OpenGL实现的版本号：%s\n", OpenGLVersion);
    //printf("OGLU工具库版本：%s\n", gluVersion);
    //如果是在VS上执行，需要在return前加上：system("pause");

    parse_argument(argc, argv);

    // : Main RayCasting Logic
    // First, parse the scene using SceneParser.
    // Then loop over each pixel in the image, shooting a ray
    // through that pixel and finding its intersection with
    // the scene.  Write the color at the intersection to that
    // pixel in your output image.

    //printf("[%d]\n", omp_get_max_threads());
    printf("Building scene...\n");
    SceneParser scene(inputFile.c_str(), AS, mode == RENDERING_MODE::PATH_TRACING_GPU, sampler2d_type);
    PerspectiveCamera* camera = dynamic_cast<PerspectiveCamera*>(scene.getCamera());
    Image img(camera->getWidth(), camera->getHeight());

    switch (mode) {
    case RENDERING_MODE::WHITTED:
        renderer = new WhittedStyleRenderer(MSAA);
        break;
    case RENDERING_MODE::PATH_TRACING_CPU:
        renderer = new PathTracer_CPU(spp, MSAA, NEE, RR);
        break;
    case RENDERING_MODE::PATH_TRACING_GPU:
        renderer = new PathTracer_GPU(spp, MSAA, AS, NEE, RR, scene.getCamera()->getWidth(), scene.getCamera()->getHeight(), window_name + " " + inputFile);
        break;
    }
#ifdef HIT_DATA
    freopen("data.txt", "w", stdout);
#endif
    // TriangleData triangleData;
    // scene.getGroup()->generateTriangleData(triangleData);

    renderer->render(scene, img);

    std::cout << "Saving BMP " << outputFile << "..." << std::endl;
    img.SaveBMP(outputFile.c_str());
#ifdef HIT_DATA
    freopen("CON", "w", stdout);
#endif
    // img.SavePPM((outputFile.substr(0, outputFile.size() - 4) + ".ppm").c_str());
    // img.SaveTGA((outputFile.substr(0, outputFile.size() - 4) + ".tga").c_str());

    std::cout << "Done." << std::endl;

    return 0;
}