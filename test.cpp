#include <atomic>
#include <iostream>

template <typename T> class Node {
public:
  T value;
  std::atomic<Node<T> *> next;
  Node(T val) : value(val), next(nullptr) {}
  Node() : next(nullptr) {}
};

/**
 * Multi-Producers and Multi-Consumers LockFreeQueue.
 * 原版本在lockfreequeue.cpp中
 */
template <typename T> class LockFreeQueue {
private:
  std::atomic<Node<T> *> head_;
  std::atomic<Node<T> *> tail_;

public:
  LockFreeQueue() {
    Node<T> *dummyNd = new Node<T>(-1);
    head_.store(dummyNd, std::memory_order_relaxed);
    tail_.store(dummyNd, std::memory_order_relaxed);
  }

  ~LockFreeQueue() {
    while (Node<T> *nd = head_.load(std::memory_order_relaxed)) {
      head_.store(nd->next.load(std::memory_order_relaxed),
                  std::memory_order_relaxed);
      delete nd;
    }
  }

  void enqueue(T value) {
    Node<T> *newNode = new Node<T> *(T);
    Node<T> *oldTail;
    Node<T> *nullNode = nullptr;
    while (true) {
      oldTail = tail_.load(std::memory_order_relaxed);
      Node<T> *nextNd = oldTail->next.load(std::memory_order_acquire);
      // 当next为nullptr时，说明没有别的线程进行入队，可以直接插入
      if (nextNd == nullptr) {
        if (oldTail->next.compare_exchange_strong(nullNode, newNode,
                                                  std::memory_order_release,
                                                  std::memory_order_relaxed)) {
          tail_.compare_exchange_strong(oldTail, newNode,
                                        std::memory_order_release,
                                        std::memory_order_relaxed);
          break;
        } else { // 期间有新的节点入队，更新tail_节点
          tail_.compare_exchange_strong(oldTail, nextNd,
                                        std::memory_order_release,
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

      if (oldHead == head_.load(std::memory_order_acquire)) {
        // 说明当前队列为空或者有一个待入栈的节点
        if (oldHead == oldTail) {
          // 说明队列为空
          if (oldTail->next == nullptr) {
            return false;
          }
          // 当前正在发生入队操作，帮助完成该操作
          tail_.compare_exchange_strong(oldTail, next,
                                        std::memory_order_release,
                                        std::memory_order_relaxed);
        } else { // 队列中存在元素
          value = next->value;
          head_.compare_exchange_strong(oldHead, next,
                                        std::memory_order_release,
                                        std::memory_order_consume);
          break;
        }
      }
    }
    delete oldHead;
    return true;
  }
};