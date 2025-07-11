cmake -B build -G "Unix Makefiles"
make -C build -j %NUMBER_OF_PROCESSORS%

@REM usage: build/RTProject.exe --input=string --output=string --rm=string [options] ...
@REM options:
@REM   -i, --input      input file (string)
@REM   -o, --output     output file (string)
@REM   -r, --rm         rendering method (string)
@REM   -d, --device     rendering device (string [=CPU])
@REM   -a, --as         acceleration structure
@REM   -s, --spp        samples per pixel (int [=1])
@REM   -m, --msaa       multi-sampling antialiasing (int [=1])
@REM   -n, --nee        next event estimation
@REM   -t, --sampler    sampler of textures (string [=Bilinear])
@REM   -p, --rr         Russian roulette (float [=0.8])
@REM   -?, --help       print this message

@REM "build/RTProject.exe" --input=testcases/scene10_teapot.txt --output=output/scene10_teapot_Whitted.bmp --rm=Whitted --as --sampler=Bilinear
@REM "build/RTProject.exe" --input=testcases/scene10_teapot.txt --output=output/scene10_teapot_Whitted.bmp --rm=RayTracing --device=GPU

@REM Whitted
@REM "build/RTProject.exe" --input=testcases/scene08_cornell_box.txt --output=output/scene08_cornell_box_Whitted.bmp --rm=Whitted --device=CPU

@REM 基本场景
@REM "build/RTProject.exe" --input=testcases/scene08_cornell_box_facelight.txt --output=output/scene08_cornell_box_facelight_refle_refra6_no_gamma.bmp --rm=RayTracing --device=CPU --spp=256 --nee --as --sampler=Bilinear
@REM "build/RTProject.exe" --input=testcases/scene10_teapot_facelight.txt --output=output/scene10_teapot_RT.bmp --rm=RayTracing --device=CPU --spp=256 --nee --as --sampler=Bilinear

@REM GPU渲染示例
@REM "build/RTProject.exe" --input=testcases/scene15_cornell_box_sphere.txt --output=output/scene15_cornell_box_sphere_RT.bmp --rm=RayTracing --device=CPU --spp=256 --as --sampler=Bilinear --nee
@REM "build/RTProject.exe" --input=testcases/scene12_Klee_facelight.txt --output=output/scene12_Klee_RT.bmp --rm=RayTracing --device=GPU --spp=-1 --nee
@REM "build/RTProject.exe" --input=testcases/scene08_cornell_box_facelight_GPU.txt --output=output/scene08_cornell_box_facelight_GPU_nee_512spp_as.bmp --rm=RayTracing --device=GPU --spp=512 --nee --as

@REM "build/RTProject.exe" --input=testcases/scene08_cornell_box_facelight_cube_light.txt --output=output/scene08_cornell_box_facelight_cube_light.bmp --rm=RayTracing --device=CPU --spp=256 --nee --as --sampler=Bilinear
@REM "build/RTProject.exe" --input=testcases/scene08_cornell_box_facelight_cube_normal_interp.txt --output=output/scene08_cornell_box_facelight_cube_ni_7.bmp --rm=RayTracing --device=CPU --spp=256 --nee --as --sampler=Bilinear
@REM "build/RTProject.exe" --input=testcases/scene08_cornell_box_facelight_no_content.txt --output=output/scene08_cornell_box_facelight_no_content3.bmp --rm=RayTracing --device=CPU --spp=128 --nee --as --sampler=Bilinear
@REM "build/RTProject.exe" --input=testcases/scene08_cornell_box_facelight_no_content_transform_light.txt --output=output/scene08_cornell_box_facelight_no_content_transform_light.bmp --rm=RayTracing --device=CPU --spp=256 --nee --as --sampler=Bilinear
@REM "build/RTProject.exe" --input=testcases/scene08_cornell_box.txt --output=output/scene08_cornell_box_facelight_refle_refra_Whitted.bmp --rm=Whitted
@REM "build/RTProject.exe" --input=testcases/scene08_cornell_box_facelight_obj_light1.txt --output=output/scene08_cornell_box_facelight_obj_light1_1.bmp --rm=RayTracing --device=CPU --spp=1024 --nee --as --sampler=Bilinear
@REM "build/RTProject.exe" --input=testcases/scene08_cornell_box_balllight.txt --output=output/scene08_cornell_box_balllight1.bmp --rm=RayTracing --device=CPU --spp=128 --nee --as --sampler=Bilinear
@REM "build/RTProject.exe" --input=testcases/scene08_cornell_box_facelight_obj_light2.txt --output=output/scene08_cornell_box_facelight_obj_light2_2.bmp --rm=RayTracing --device=GPU --spp=1024 --nee --as --sampler=Bilinear

@REM 复杂场景1：纹理贴图+法线贴图
@REM "build/RTProject.exe" --input=testcases/scene13_eagle_facelight.txt --output=output/scene13_eagle_RT_normal_texture_transform.bmp --rm=RayTracing --device=CPU --spp=32 --as --sampler=Bilinear --nee

@REM 复杂场景2：多纹理贴图+法线插值
@REM "build/RTProject.exe" --input=testcases/scene12_Klee_facelight.txt --output=output/scene12_Klee_RT.bmp --rm=RayTracing --device=CPU --spp=256 --as --sampler=Bilinear --nee
@REM "build/RTProject.exe" --input=testcases/scene12_Klee_facelight_normal_interp.txt --output=output/scene12_Klee_RT_normal_interp.bmp --rm=RayTracing --device=CPU --spp=128 --as --sampler=Bilinear --nee

@REM "build/RTProject.exe" --input=testcases/scene12_Klee_self_light.txt --output=output/scene12_Klee_self_light.bmp --rm=RayTracing --device=CPU --spp=256 --as --sampler=Bilinear --nee

@REM "build/RTProject.exe" --input=testcases/scene14_roadBike_facelight.txt --output=output/scene14_roadBike_10x_RT.bmp --rm=RayTracing --device=CPU --spp=128 --as --sampler=Bilinear --nee
@REM "build/RTProject.exe" --input=testcases/scene15_cornell_box_sphere.txt --output=output/scene15_cornell_box_sphere_RT.bmp --rm=RayTracing --device=CPU --spp=256 --as --sampler=Bilinear --nee
@REM "build/RTProject.exe" --input=testcases/scene16_bunny.txt --output=output/scene16_bunny_1k_6x_GPU_RT_64spp_no_gamma_N.bmp --rm=RayTracing --device=GPU --spp=64 --nee --as
@REM "build/RTProject.exe" --input=testcases/scene16_bunny1.txt --output=output/scene16_bunny1_12x_CPU_RT_64spp.bmp --rm=RayTracing --device=CPU --spp=64 --nee --as

@REM 报告
@REM teaser
"build/RTProject.exe" --input=testcases/teaser.txt --output=output/teaser.bmp --rm=RayTracing --device=CPU --spp=256 --as --sampler=Bilinear --nee
@REM 一、Whitted-Style光线追踪
@REM "build/RTProject.exe" --input=testcases/scene08_cornell_box.txt --output=output/scene08_cornell_box_Whitted.bmp --rm=Whitted
@REM 二、路径追踪
@REM "build/RTProject.exe" --input=testcases/scene08_cornell_box_facelight.txt --output=output/scene08_cornell_box_facelight_128spp_cos-weighted.bmp --rm=RayTracing --device=CPU --spp=128 --as --sampler=Bilinear --nee
@REM "build/RTProject.exe" --input=testcases/scene17_glossy.txt --output=output/scene17_glossy_0.5r_0.4m.bmp --rm=RayTracing --device=CPU --spp=128 --nee --as
@REM 三、加速结构
@REM "build/RTProject.exe" --input=testcases/scene16_bunny.txt --output=output/scene16_bunny_1k_128spp.bmp --rm=RayTracing --device=CPU --spp=128 --nee --as
@REM 四、GPU渲染
@REM "build/RTProject.exe" --input=testcases/scene16_bunny.txt --output=output/scene16_bunny_1k_Intel_64spp.bmp --rm=RayTracing --device=GPU --spp=64 --nee --as
@REM "build/RTProject.exe" --input=testcases/scene16_bunny.txt --output=output/scene16_bunny_1k_CPU_64spp.bmp --rm=RayTracing --device=CPU --spp=64 --nee --as
@REM 五、cos-weight采样
@REM "build/RTProject.exe" --input=testcases/scene16_bunny.txt --output=output/scene16_bunny_1k_64spp.bmp --rm=RayTracing --device=CPU --spp=64 --nee --as
@REM 六、MSAA
@REM "build/RTProject.exe" --input=testcases/scene08_cornell_box_facelight.txt --output=output/scene08_cornell_box_facelight_16msaa.bmp --rm=RayTracing --device=CPU --spp=1024 --as --nee --sampler=Bilinear --msaa=16
@REM "build/RTProject.exe" --input=testcases/scene08_cornell_box_facelight.txt --output=output/scene08_cornell_box_facelight_4msaa.bmp --rm=RayTracing --device=CPU --spp=1024 --as --nee --sampler=Bilinear --msaa=4
@REM "build/RTProject.exe" --input=testcases/scene08_cornell_box_facelight.txt --output=output/scene08_cornell_box_facelight_no_msaa.bmp --rm=RayTracing --device=CPU --spp=1024 --as --nee --sampler=Bilinear
@REM 七、纹理贴图
@REM "build/RTProject.exe" --input=testcases/scene13_eagle_facelight.txt --output=output/scene13_eagle_no_diff.bmp --rm=RayTracing --device=CPU --spp=128 --as --sampler=Bilinear --nee
@REM 八、法线插值
@REM "build/RTProject.exe" --input=testcases/scene12_Klee_facelight_normal_interp.txt --output=output/scene12_Klee_RT_normal_interp.bmp --rm=RayTracing --device=CPU --spp=128 --as --sampler=Bilinear --nee
@REM 九、Gamma校正
@REM "build/RTProject.exe" --input=testcases/scene08_cornell_box_facelight.txt --output=output/scene08_cornell_box_facelight_no_gamma.bmp --rm=RayTracing --device=CPU --spp=128 --as --nee --sampler=Bilinear
@REM 十、OMP加速
@REM "build/RTProject.exe" --input=testcases/scene08_cornell_box_facelight.txt --output=output/scene08_cornell_box_facelight_no_omp.bmp --rm=RayTracing --device=CPU --spp=128 --as --nee --sampler=Bilinear