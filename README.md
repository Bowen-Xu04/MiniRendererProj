# MiniRendererProj
This is a minimal renderer based on my project of the course "Fundamentals of Computer Graphics", in which I got a grade of A. I am continuously removing bugs and developing new characteristics of this project. Feel free to try it!



<img src=".\img\【7】scene13_eagle.bmp" alt="【7】scene13_eagle" style="zoom: 15%;" />



### Environment

The code can run normally in Windows, but I am not sure whether it can also run in Linux/MacOS.



### Dependency

cmdline

glad

glfw-3.4

stb_image

tinyobjloader



### Features

1. Whitted-Style ray tracing;
2. Path tracing;
3. Physically based rendering (glossy BRDF);
4. (developing) *GPU parallel acceleration based on OpenGL*;
5. (developing) Importance sampling: cos-weighted, *BRDF*, *MIS*;
6. MSAA;
7. Diffuse texture and normal texture for triangular meshes;
8. Normal interpolation;
9. Gamma correction;
10. CPU parallel acceleration based on OpenMP.
