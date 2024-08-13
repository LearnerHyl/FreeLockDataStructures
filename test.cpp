#include <iostream>
#include <atomic>

template<typename T>
class Node{
public:
  T val_;
  std::atomic<Node<T>*> next_;
  Node(T val): val_(val), next_(nullptr) {}
  Node():next_(nullptr) {}
};

template<typename T>
class LockFreeQueue{
public:
  LockFreeQueue() {
    Node<T>* dummy = new Node<T> (-1);
    // relax语义
    head_.store(dummy);
    head_.store(dummy);
  }

  ~LockFreeQueue() {
    // relax语义
    while (Node<T>* nd = head_.load()) {
      head_.store(nd->next_);
      delete nd;
    }
  }

  void enqueue(T val) {
    Node<T>* nd = new Node<T>(val);
    Node<T>* oldTail;

    while (true) {
      // relax语义
      oldTail = tail_.load();
      // acquire语义
      Node<T>* next = oldTail->next_.load();
      // acquire语义，tail指针一致可以进行下一步
      if (tail_.load() == oldTail) {
        // 没有新的节点正在插入
        if (next == nullptr) {
          // 插入新节点, release语义
          if (oldTail->next_.compare_exchange_strong(nullptr, nd)) {
            tail_.compare_exchange_strong(oldTail, nd);
            break;
          }
        } else {
          // 说明有正在插入的节点,帮助其完成插入, release语义
          tail_.compare_exchange_strong(oldTail, next);
        }
      }
    }
  }

  bool dequeue(T& val) {
    Node<T>* oldHead;
    Node<T>* oldTail;
    Node<T>* next;
    while (true) {
      // relaxed语义
      oldHead = head_.load();
      oldTail = head_.load();
      // acquire语义
      next = oldHead->next_.load();
      if (oldHead == oldTail) {
        // 队列为空
        if (next == nullptr) {
          return false;
        }
        // 帮助其完成插入, release语义
        tail_.compare_exchange_strong(oldTail, next);
      } else { // 存在节点
        val = next->val_;
        // release语义
        head_.compare_exchange_strong(oldHead, next);
        delete oldHead;
        break;
      }
    }
    return true;
  }

private:
  std::atomic<Node<T>*> head_;
  std::atomic<Node<T>*> tail_;
};