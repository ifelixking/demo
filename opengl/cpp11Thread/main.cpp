#include <iostream>
#include <thread>

void func1() {
    for (int i = 0; i < 100000000; ++i) {
        std::cout << "thread1: " << i << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void func2() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "thread 2 exit" << std::endl;
}

void func3() {
    for (int i = 0; i < 5; ++i) {
        std::cout << "thread3:" << i << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main() {

    // 获取CPU个数
    auto core = std::thread::hardware_concurrency();
    std::cout << "CPU count: " << core << std::endl;

    // std::thread 的 析构函数 会检查是否能join，如果能，就会调用 std::terminater, 并会抛出异常
    // 也就是说， std::thread 析构时，线程要么被 join（意味着已经结束），要么被 detach将其放飞成为守护线程
    // std::thread 貌似没法做到 像 Java中那样，主线程退出，子线程继续运行
    // 如果要到达这种效果。linux下需要在主线程中调用 pthread_exit
    {
        std::thread k(func3);
        k.detach();
    }

    // 启动线程，并使它成为守护线程，主线程退出，守护线程也会退出
    std::thread thread1(func1);
    thread1.detach();

    // 启动线程并等待结束
    std::thread thread2(func2);
    thread2.join();

    // 注：thread3析构会报错，原因如上描述
    std::thread thread3 (func3);

    std::cout << "main exit" << std::endl;
    return 0;
}

// ========================================================================
void * func_long(void *) {
    for (int i = 0; i < 100000000; ++i) {
        std::cout << "thread1: " << i << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void * func_print(void *) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "thread 2 exit" << std::endl;
}

void * func_awhile(void *) {
    for (int i = 0; i < 5; ++i) {
        std::cout << "thread3:" << i << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main_pthread(){

    // 创建子线程
    pthread_t thread1;
    pthread_create(&thread1, nullptr, func_awhile, nullptr);

    std::cout<<"main exiting"<<std::endl;

    // pthread_exit 会立刻使当前线程退出，不执行后面的代码，但不会导致子线程退出
    pthread_exit(nullptr);
    std::cout<<"main exit"<<std::endl;

    return 0;
}