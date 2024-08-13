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
    ThreadPrinter(int printSum, int threadNum): printSum_(printSum), threadNum_(threadNum) {

    }

    void PrintNumber() {
        int threadId = curThreadId_++;
        while (curPrintVal_ <= printSum_) {
            if (curPrintVal_ % threadNum_ == threadId) {
                std::cout << "Thread:" << threadId << " print number:" << curPrintVal_++ << std::endl;
            }
            // 让出CPU给别的线程
            std::this_thread::yield();
        }
    }

    void StartPrinter() {
        std::vector<std::thread> threadSet;
        for (int i = 0; i < threadNum_; i++) {
            threadSet.emplace_back(&ThreadPrinter::PrintNumber, this);
        }

        // 等待线程退出
        for (auto &thd : threadSet) {
            thd.join();
        }
    }

private:
    std::atomic<int> curThreadId_{0};
    std::atomic<int> curPrintVal_{0};
    int printSum_;
    int threadNum_;
};

int main() {
    int N = 4, M = 9;
    ThreadPrinter tp(N, M);
    tp.StartPrinter();
    return 0;
}