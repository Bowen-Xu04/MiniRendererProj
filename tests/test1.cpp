#include <iostream>
#include <cassert>
#include <exception>
#include <omp.h>

void fff(int percent) {
    if (percent < 0) {
        throw std::runtime_error("ddss");
    }
}

void upd(int percent) {
    fff(percent);
    printf("hhh.\n");
    int barWidth = 100;
    printf("Progress: [");
    int pos = barWidth * percent / 100;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) putchar('=');
        else if (i == pos) putchar('>');
        else putchar(' ');
    }
    printf("] %d%%\n", percent);
}

int main() {
    printf(".\n");

    try {
        upd(-1);
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        assert(0);
    }

    const int total = 1000000;
    int progress = 0;
    int percent = 0;
#pragma omp parallel for num_threads(omp_get_max_threads()) shared(progress)
    for (int i = 0; i < total; ++i) {
        // 模拟计算任务
        for (int j = 0; j < 1000; ++j) {}

        // 原子更新进度
#pragma omp atomic
        progress++;

        // 通过 critical 区域限制输出，并仅由主线程操作
#pragma omp critical
        {
            if (omp_get_thread_num() == 0 && progress * 100 / total > percent) {
                //float percent = static_cast<float>(progress) / total * 100;
                ++percent;
                upd(percent);
                //printf("Progress: %d%%\r", percent);
                //std::cout << "\rProgress: " << percent << "% " << std::flush;
            }
        }
    }
    upd(100);
    printf("\n");
    // percent = 100;
    // int barWidth = 100;
    // printf("Progress: [");
    // int pos = barWidth * percent / 100;
    // for (int i = 0; i < barWidth; ++i) {
    //     if (i < pos) putchar('=');
    //     else if (i == pos) putchar('>');
    //     else putchar(' ');
    // }
    // printf("] %d%%\n", percent);
    //std::cout << "\rProgress: 100%     \n";
    return 0;
}