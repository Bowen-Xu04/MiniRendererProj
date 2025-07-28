# MiniRendererProj
**==Update: 2025.7.28==**

1. Developed importance sampling: BRDF and MIS;

2. Fixed some bugs.

---

This is a minimal renderer based on my project of the course "Fundamentals of Computer Graphics", in which I got a grade of A (project grade: 42.5/45). I am continuously removing bugs and developing new characteristics of this project. Feel free to try it!

**Note: For more details about features and analyses of rendering results , see the document PDF (only Chinese version).** 

<br/>

<p align="center">
<img src="img\scene21_c_sponza_MIS_1280x800_512spp.bmp" alt="scene21_c_sponza_MIS_1280x800_512spp" width="384" />
</p>

<p align="center">
Crytek Sponza (512spp, rendering time: 7137s)<br/>
Features: MIS, diffuse texture, normal interpolation
</p>

<br/>

<p align="center">
<img src="img\teaser_test_1024x1024_MIS_512spp.bmp" alt="teaser_test_1024x1024_MIS_512spp" width="300" />
</p>

<p align="center">
Cornell Box (512spp, rendering time: 1377s)<br/>
Features: MIS, diffuse texture, normal texture, normal interpolation, glossy material, refractive material
</p>

<br/>

<p align="center">
<img src="img\scene19_BMW_MIS_1280x800_1024spp.bmp" alt="scene19_BMW_MIS_1280x800_1024spp" width="384" />
</p>

<p align="center">
BMW in the Auto Show (1024spp, rendering time: 3344s)<br/>
Features: MIS, diffuse texture, normal interpolation, reflective material, mesh light
</p>

<br/>


### Environment

The code can run normally on Windows, but I am not sure whether it can also run on Linux/MacOS.

<br/>

### Dependency

cmdline

glad

glfw-3.4

stb_image

tinyobjloader

<br/>

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
