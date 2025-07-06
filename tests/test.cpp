#include <iostream>
#include <thread>
#include <chrono>
#include <omp.h>

int foo(int x) {
    return x;
}

int main() {
    const int total = 1000000000;
    int progress = 0;
    bool a = true;
    std::cout << a << std::endl;
    return 0;
#pragma omp parallel shared(progress) num_threads(16)
    {

        // 并行执行计算任务
#pragma omp for nowait
        for (int i = 0; i < total; ++i) {
            int temp = foo(i);  // 模拟计算
            //putchar('.');
#pragma omp atomic
            progress++;
        }

        // 主线程单独更新进度条
#pragma omp master
        {
            while (progress < total) {
                float percent = static_cast<float>(progress) / total * 100;
                std::cout << "\rProgress: " << percent << "% ";
                // 适当休眠减少轮询开销
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            std::cout << "\rProgress: 100%     \n";
        }
    }
    return 0;
}