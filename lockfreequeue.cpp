#include <atomic>
#include <iostream>

/**
 * 多生产者多消费者无锁队列，是一个性能优化的版本，
 * 因为严格使用了内存序列，而不是都采用最高级别的内存序列。
 */

// g++ lockfreequeue.cpp -o lfq
template <typename T> class Node {
public:
  T value; // 节点存储的值
  std::atomic<Node<T> *> next; // 指向下一个节点的指针，使用原子操作保证线程安全

  Node(T val) : value(val), next(nullptr) {}
  Node() : next(nullptr) {}
};

template <typename T> class LockFreeQueue {
private:
  std::atomic<Node<T> *> head; // 队列头指针
  std::atomic<Node<T> *> tail; // 队列尾指针

public:
  LockFreeQueue() {
    Node<T> *dummy = new Node<T>(); // 创建一个哑元节点，初始化队列时队列为空，头尾指针指向这个哑元节点
    head.store(dummy, std::memory_order_relaxed);
    tail.store(dummy, std::memory_order_relaxed);
  }

  ~LockFreeQueue() {
    while (Node<T> *node = head.load(
               std::memory_order_relaxed)) { // 在析构函数中清理所有节点
      head.store(node->next.load(std::memory_order_relaxed),
                 std::memory_order_relaxed);
      delete node;
    }
  }

  void enqueue(T value) {
    Node<T> *newNode = new Node<T>(value); // 创建新节点
    Node<T> *oldTail;
    Node<T> *nullNode = nullptr;

    while (true) {
      oldTail = tail.load(std::memory_order_relaxed);
      Node<T> *next = oldTail->next.load(std::memory_order_acquire);
      if (oldTail == tail.load(std::memory_order_acquire)) {
        if (next == nullptr) { // 如果尾部的next为空，尝试将新节点链接到尾部
          // 为什么选择这样的内存顺序？
          // 成功的情况下使用
          // release：当你添加一个新节点到队列的尾部时，你希望确保节点正确地初始化，并且在其他线程看到这个节点之前，这些初始化都已经完成了。这里的关键是保证数据的完整性和正确性。
          // 失败的情况下使用
          // relaxed：如果更新失败了，实际上并没有对数据结构造成影响，因此不需要强制对内存顺序进行控制。失败仅意味着本次操作没有改变任何状态，可以减少同步的开销。
          // 这种使用内存屏障的策略是无锁编程中常见的优化手段，旨在在保证必要的同步和数据一致性的同时，尽可能减小同步操作的性能开销。这种细致的内存顺序控制允许构建高性能的并发数据结构。
          if (oldTail->next.compare_exchange_weak(nullNode, newNode,
                                                  std::memory_order_release,
                                                  std::memory_order_relaxed)) {
            tail.compare_exchange_strong(oldTail, newNode,
                                         std::memory_order_release,
                                         std::memory_order_relaxed);
            break; // 成功插入新节点，退出循环
          }
        } else { // 如果尾部next不为空，说明尾部已经被其他线程更新，尝试推进尾部指针
          tail.compare_exchange_strong(oldTail, next, std::memory_order_release,
                                       std::memory_order_relaxed);
        }
      }
    }
  }

  bool dequeue(T &value) {
    Node<T> *oldHead;
    Node<T> *oldTail;
    Node<T> *next;

    while (true) {
      oldHead = head.load(std::memory_order_relaxed); // 读取头指针
      oldTail = tail.load(std::memory_order_relaxed); // 读取尾指针
      next = oldHead->next.load(std::memory_order_acquire);

      if (oldHead == head.load(std::memory_order_acquire)) {
        if (oldHead == oldTail) { // 检查队列是否为空或尾部落后
          // 在无锁队列中，头指针（head）和尾指针（tail）用于标记队列的开始和结束。由于出队操作和入队操作可能同时发生，
          // 队列可能会出现一种状态：头指针和尾指针相等，
          // 但队列并不为空（即有节点在尾指针的后面）。这种情况可能是由于入队操作还没有完全完成。
          if (next == nullptr)
            return false; // 队列为空
          // 帮助队列完成未完成的入队操作
          tail.compare_exchange_strong(
              oldTail, next, std::memory_order_release,
              std::memory_order_relaxed); // 尝试推进尾部指针
        } else {
          value = next->value; // 读取下一个节点的值
          if (head.compare_exchange_weak(
                  oldHead, next, std::memory_order_release,
                  std::memory_order_relaxed)) // 尝试推进头指针
            break;                            // 成功出队，退出循环
        }
      }
    }
    delete oldHead; // 释放旧的头节点
    return true;
  }
};

int main() {
  LockFreeQueue<int> queue;

  queue.enqueue(1);
  queue.enqueue(2);
  queue.enqueue(3);

  int value;
  while (queue.dequeue(value)) {
    std::cout << "Dequeued: " << value << std::endl;
  }

  return 0;
}
