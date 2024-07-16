#include <iostream>
#include <thread>
#include <atomic>
#include <vector>

/**
 * N个线程，数大小为M，要求N个线程依次打印0-M。
*/

// g++ threadprinter.cpp -o tp
class ThreadPrinter{
public:
    ThreadPrinter(int threadNum, int printSum):threadNum_(threadNum), printSum_(printSum) {

    }

    void PrintNumber() {
        int curThreadId = nextThreadId_++;
        while (currentNum_ < printSum_) {
            if (currentNum_ % threadNum_ == curThreadId) {
                std::cout << "Thread Id " << curThreadId << "print number" << currentNum_++ << std::endl;
            }
            // 否则让出CPU给其他的线程
            std::this_thread::yield();
        }
    }

    void StartPrinter() {
        std::vector<std::thread> threadSet;
        // 初始化线程池
        for (int i = 0; i < threadNum_; i++) {
            threadSet.emplace_back(&ThreadPrinter::PrintNumber, this);
        }

        // 等待所有的线程依次退出
        for (auto &thd : threadSet) {
            thd.join();
        }
    }

private:
    std::atomic<int> nextThreadId_{0};
    std::atomic<int> currentNum_{0};
    int threadNum_;
    int printSum_;
};

int main() {
    int N = 4, M = 9;
    ThreadPrinter tp(N, M);
    tp.StartPrinter();
    return 0;
}