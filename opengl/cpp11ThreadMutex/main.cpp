#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>

const long int SUM = 10000000;


int main() {

    // 没有加锁，result的值最终可能不是SUM*2
    {
        long int result = 0;
        auto func = [&result](){
            for (long int i = 0; i < SUM; ++i) {
                ++result;
            }
        };
        auto tStart = clock();
        std::thread thread1(func);
        std::thread thread2(func);
        thread1.join();
        thread2.join();
        auto tEnd = clock();
        std::cout << "SUM: " << SUM << "\t" << "result: " << result << std::endl;
        std::cout << (float) (tEnd - tStart) / CLOCKS_PER_SEC << std::endl;
    }

    // 加锁，结果正确，但效率不高
    {
        long int result = 0;
        std::mutex lock;
        auto func = [&lock, &result](){
            for (long int i = 0; i < SUM; ++i) {
                lock.lock();
                ++result;
                lock.unlock();
            }
        };
        auto tStart = clock();
        std::thread thread1(func);
        std::thread thread2(func);
        thread1.join();
        thread2.join();
        auto tEnd = clock();
        std::cout << "SUM: " << SUM << "\t" << "result: " << result << std::endl;
        std::cout << (float) (tEnd - tStart) / CLOCKS_PER_SEC << std::endl;
    }

    // 使用 atom, 比使用 lock 效率要高一倍以上
    {
        std::atomic_llong result; result = 0;
        auto func = [&result](){
            for (long int i = 0; i < SUM; ++i) {
                ++result;
            }
        };
        auto tStart = clock();
        std::thread thread1(func);
        std::thread thread2(func);
        thread1.join();
        thread2.join();
        auto tEnd = clock();
        std::cout << "SUM: " << SUM << "\t" << "result: " << result << std::endl;
        std::cout << (float) (tEnd - tStart) / CLOCKS_PER_SEC << std::endl;
    }




    return 0;
}