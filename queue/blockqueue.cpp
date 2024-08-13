#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template <typename T>
class BlockQueue {
public:
    BlockQueue(size_t max_size) : max_size_(max_size) {}

    // 添加元素到队列中，如果队列已满则阻塞
    void put(const T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_put_.wait(lock, [this]() { return queue_.size() < max_size_; });
        queue_.push(item);
        cond_get_.notify_one();
    }

    // 从队列中取出元素，如果队列为空则阻塞
    T get() {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_get_.wait(lock, [this]() { return !queue_.empty(); });
        T item = queue_.front();
        queue_.pop();
        cond_put_.notify_one();
        return item;
    }

private:
    std::queue<T> queue_;
    size_t max_size_;
    std::mutex mutex_;
    std::condition_variable cond_put_;
    std::condition_variable cond_get_;
};

// 测试生产者函数
void producer(BlockQueue<int>& bq, int start, int end) {
    for (int i = start; i <= end; ++i) {
        std::cout << "生产者生产: " << i << std::endl;
        bq.put(i);
    }
}

// 测试消费者函数
void consumer(BlockQueue<int>& bq, int consume_count) {
    for (int i = 0; i < consume_count; ++i) {
        int item = bq.get();
        std::cout << "消费者消费: " << item << std::endl;
    }
}

int main() {
    BlockQueue<int> bq(5);  // 创建最大容量为5的阻塞队列

    std::thread producer_thread(producer, std::ref(bq), 1, 10);
    std::thread consumer_thread(consumer, std::ref(bq), 10);

    producer_thread.join();
    consumer_thread.join();

    return 0;
}
