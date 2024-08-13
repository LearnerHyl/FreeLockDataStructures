#include <iostream>
#include <atomic>

template<typename T>
struct Node {
  int val;
  std::atomic<Node<T> *> next;
  Node(int value):val(value), next(nullptr) {}
  Node():next(nullptr) {}
};

template<typename T>
class LockFreeQueue{
private:
  std::atomic<Node<T>*> head_;
  std::atomic<Node<T>*> tail_;

public:
  LockFreeQueue() {
    Node<T>* dummy = new Node<T>();
    // 使用relax语义
    head_.store(dummy);
    tail_.store(dummy);
  }

  ~LockFreeQueue() {
    while (Node<T> *nd = head_.load()) {
      // relex语义
      head_ = nd->next.load();
      delete nd;
    }
  }

  void enqueue(T val) {
    Node<T>* nd = new Node<T>(val);
    Node<T>* oldTail;
    Node<T>* oldNext;
    
    while (true) {
      // relax
      oldTail = tail_.load();
      // acquire
      oldNext = oldTail->next.load();
      // acquire
      if (oldTail == tail_.load()) { // 若tail_没变，尝试插入数据
        // oldTail的next若为nullptr，可以尝试插入数据
        // 成功用store,失败用relax
        if (oldTail->next.compare_exchange_strong(nullptr, nd)) {
          // 尝试更新Tail指针,成了用store,失败用relax
          tail_.compare_exchange_strong(oldTail, nd);
          break;
        } else { // next已经存在数据，帮助其更新tail指针
          // 同理，成功用store,失败用relax
          tail_.compare_exchange_strong(oldTail, oldNext);
        }
      }
    }
  }

  bool dequeue(T &val) {
    Node<T>* oldHead;
    Node<T>* oldTail;
    Node<T>* oldNext;

    while (true) {
      // relax语义
      oldHead = head_.load();
      oldTail = tail_.load();
      // acquire语义
      oldNext = oldHead->next.load();

      // acquire语义
      // 首先要确保head没变
      if (oldHead == head_.load()) {
        // 队列可能为空或者有一个正在插入的节点
        if (oldHead == oldTail) {
          // 队列为空，直接返回false
          if (oldNext == nullptr) {
            return false;
          }
          // 帮助完成未完成的入队操作，更新尾部指针
          tail_.compare_exchange_strong(oldTail, oldNext);
        } else { // 队列有元素，可以正常出队
          val = oldNext->val;
          // 成功用store,失败用relax
          head_.compare_exchange_strong(oldHead, oldNext);
        }
      }
    }
    delete oldHead;
    return true;
  }
};