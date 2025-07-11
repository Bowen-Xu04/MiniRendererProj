CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(RTProject C CXX)

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release)
ENDIF()

ADD_SUBDIRECTORY(deps/vecmath)

SET(RTProject_SOURCES
        src/glsl.cpp
        src/image.cpp
        src/main.cpp
        src/material.cpp
        src/mesh.cpp
        src/object3d.cpp
        src/scene_parser.cpp
        src/texture.cpp
        src/utils.cpp)

SET(RTProject_INCLUDES
        include/AABB.hpp
        include/BVH.hpp
        include/camera.hpp
        include/curve.hpp
        include/glsl.hpp
        include/group.hpp
        include/hit.hpp
        include/image.hpp
        include/light.hpp
        include/material.hpp
        include/mesh.hpp
        include/object3d.hpp
        include/plane.hpp
        include/ray.hpp
        include/renderer.hpp
        include/revsurface.hpp
        include/sampler.hpp
        include/scene_parser.hpp
        include/shader.hpp
        include/sphere.hpp
        include/texture.hpp
        include/transform.hpp        
        include/triangle.hpp
        include/utils.hpp
        )

INCLUDE_DIRECTORIES("./deps")
INCLUDE_DIRECTORIES("./deps/glad")
INCLUDE_DIRECTORIES("./deps/glfw-3.4/include")
INCLUDE_DIRECTORIES("./shaders")

set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)

if(UNIX)
	find_package(glfw3 REQUIRED)
    set(_GLFW_ "glfw")
elseif(WIN32)
    set(_GLFW_ "glfw3.lib")
else()
	message("ERROR: Unknown platform!")
endif()

find_package(OpenMP REQUIRED)
if(OpenMP_FOUND)
    message(STATUS "Found OpenMP.") 
    set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} ${OPENMP_C_FLAGS} "-fopenmp")
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${OPENMP_CXX_FLAGS} "-fopenmp")
endif()

add_library(glad STATIC deps/glad/glad.c)

SET(CMAKE_CXX_STANDARD 17)

ADD_EXECUTABLE(${PROJECT_NAME} ${RTProject_SOURCES} ${RTProject_INCLUDES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} vecmath glad ${_GLFW_} ${CMAKE_DL_LIBS})
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE include ${glfw3_INCLUDE_DIRS} ${GLAD_INCLUDE_DIR})