# MiniRendererProj
**==Update: 2025.7.28==**

1. Developed importance sampling: BRDF and MIS;

2. Fixed some bugs.

---

This is a minimal renderer based on my project of the course "Fundamentals of Computer Graphics", in which I got a grade of A (project grade: 42.5/45). I am continuously removing bugs and developing new characteristics of this project. Feel free to try it!

**Note: For more details about features and analyses of rendering results , see the document PDF (only Chinese version).** 



<img src="E:\THU-Programming\Fundamentals of Computer Graphics\Project_2022012236\MiniRendererProj\img\scene21_c_sponza_MIS_1280x800_512spp.bmp" alt="scene21_c_sponza_MIS_1280x800_512spp" style="zoom: 33%;" />

<center><div>Crytek Sponza (512spp, rendering time: 7137s)</div><div>Features: MIS, diffuse texture, normal interpolation</div></center>



<img src="E:\THU-Programming\Fundamentals of Computer Graphics\Project_2022012236\MiniRendererProj\img\teaser_test_1024x1024_MIS_512spp.bmp" alt="teaser_test_1024x1024_MIS_512spp" style="zoom: 33%;" />

<center><div>Cornell Box (512spp, rendering time: 1377s)</div><div>Features: MIS, diffuse texture, normal texture, normal interpolation, glossy material, refractive material</div></center>



<img src="E:\THU-Programming\Fundamentals of Computer Graphics\Project_2022012236\MiniRendererProj\img\scene19_BMW_MIS_1280x800_1024spp.bmp" alt="scene19_BMW_MIS_1280x800_1024spp" style="zoom: 33%;" />

<center><div>BMW in the Auto Show (1024spp, rendering time: 3344s)</div><div>Features: MIS, diffuse texture, normal interpolation, reflective material, mesh light</div></center>



### Environment

The code can run normally on Windows, but I am not sure whether it can also run on Linux/MacOS.



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
4. (developing) GPU parallel acceleration based on OpenGL, supporting Intel and *NVIDIA* graphics card;
5. Importance sampling: cos-weighted, BRDF, MIS;
6. MSAA;
7. Diffuse texture and normal texture for triangular meshes;
8. Normal interpolation;
9. Gamma correction;
10. CPU parallel acceleration based on OpenMP.
