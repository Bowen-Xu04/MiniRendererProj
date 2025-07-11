// 参考已有代码：https://learnopengl-cn.github.io/01%20Getting%20started/05%20Shaders/
#ifndef SHADER_H
#define SHADER_H

#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include "utils.hpp"

class Shader {
private:
    unsigned int shaderID;
    unsigned int type;

    std::string filename;
    const char* source;

public:
    Shader() = delete;

    Shader(std::string _filename, unsigned int _type) : shaderID(glCreateShader(_type)), type(_type), filename(_filename), source(load()) {
        compile();
    }

    ~Shader() {
        if (source != nullptr) {
            delete[] source;
        }
    }

    unsigned int get_id() const {
        return shaderID;
    }

    const char* get_source() const {
        if (source == nullptr) {
            printf("ERROR: No shader source code.\n");
            exit(1);
        }

        return source;
    }

private:
    const char* load() { // 参考已有代码
        std::ifstream file(filename);
        if (!file) {
            std::cout << "ERROR: Cannot read shader source code " << filename << std::endl;
            exit(1);
        }

        std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        int len = str.length();

        char* _source = new char[len + 1];
        strcpy(_source, str.c_str());
        _source[len] = '\0';

        return _source;
    }

    void compile() {
        glShaderSource(shaderID, 1, &source, NULL);
        glCompileShader(shaderID);

        checkError();
    }

    void checkError() {
        int  success = 0;
        char infoLog[512];
        memset(infoLog, 0, sizeof(infoLog));

        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
            std::cout << "ERROR: Failed to compile shader " << filename << "." << std::endl << infoLog << std::endl;
            exit(1);
        }
    }
};

class ShaderProgram {
private:
    unsigned int programID;
    Shader* vertexShader, * fragmentShader;

public:
    ShaderProgram() = delete;

    ShaderProgram(std::string vs_filename, std::string fs_filename) : programID(glCreateProgram()),
        vertexShader(new Shader(vs_filename, GL_VERTEX_SHADER)),
        fragmentShader(new Shader(fs_filename, GL_FRAGMENT_SHADER)) {
        link();
    }

    ~ShaderProgram() {
        if (vertexShader != nullptr) {
            glDeleteShader(vertexShader->get_id());
            delete vertexShader;
        }
        if (fragmentShader != nullptr) {
            glDeleteShader(fragmentShader->get_id());
            delete fragmentShader;
        }
    }

    unsigned int get_id() const {
        return programID;
    }

    void use() {
        glUseProgram(programID);
    }

    void set(const char* key, const int value) {
        int location = glGetUniformLocation(programID, key);
        glUseProgram(programID);
        glUniform1i(location, value);
    }

    void set(const char* key, const float value) {
        int location = glGetUniformLocation(programID, key);
        glUseProgram(programID);
        glUniform1f(location, value);
    }

    void set(const char* key, const Vector3f& value) {
        int location = glGetUniformLocation(programID, key);
        glUseProgram(programID);
        glUniform3f(location, value.x(), value.y(), value.z());
    }

    void set(const char* key, const Vector4f& value) {
        int location = glGetUniformLocation(programID, key);
        glUseProgram(programID);
        glUniform4f(location, value.x(), value.y(), value.z(), value.w());
    }

    void set(const char* key, const GLfloat* value) {
        int location = glGetUniformLocation(programID, key);
        glUseProgram(programID);
        glUniformMatrix3fv(location, 1, GL_FALSE, value);
    }

private:
    void link() {
        glAttachShader(programID, vertexShader->get_id());
        glAttachShader(programID, fragmentShader->get_id());
        glLinkProgram(programID);

        checkError();
    }

    void checkError() {
        int success = 0;
        char infoLog[512] = {};
        glGetProgramiv(programID, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(programID, 512, NULL, infoLog);
            std::cout << "ERROR: Failed to link shader program." << std::endl << infoLog << std::endl;
        }
    }
};

#endif