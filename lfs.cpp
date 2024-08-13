#include <iostream>
#include <atomic>

/**
 * 实现一个LockFreeStack
 */
template <typename T>
class Node{
public:
  T val_;
  Node<T> *next;
  Node(T value) : val_(value), next(nullptr) {}
  Node() : next(nullptr) {}
};

template <typename T>
class LockFreeStack{
private:
  std::atomic<Node<T> *> head_;

public:
  LockFreeStack() {
    Node<T> *dummy = new Node<T> (-1);
    head_.store(dummy);
  }

  ~LockFreeStack() {
    while (Node<T> *nd = head_.load()) {
      head_.store(nd->next);
      delete nd;
    }
  }

  void Push(T value) {
    Node<T> *nd = new Node<T> (value);
    nd->next = head_.load();

    while (!head_.compare_exchange_strong(nd->next, nd));
  }

  bool Pop(T &value) {
    Node<T> *oldHead = head_.load();
    while (oldHead && !head_.compare_exchange_strong(oldHead, oldHead->next));
    if (oldHead == nullptr) {
      return false;
    }
    value = oldHead->val_;
    delete oldHead;
    return true;
  }

};