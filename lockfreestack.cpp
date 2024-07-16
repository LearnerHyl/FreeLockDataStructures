#include <atomic>
#include <iostream>

template<typename T>
class Node {
public:
    T value;
    Node* next;

    Node(T val) : value(val), next(nullptr) {}
};

template<typename T>
class LockFreeStack {
private:
    std::atomic<Node<T>*> head;  // 栈顶指针

public:
    LockFreeStack() : head(nullptr) {}

    ~LockFreeStack() {
        Node<T>* node;
        while (node = head.load(std::memory_order_relaxed)) {  // 清理所有节点
            head.store(node->next, std::memory_order_relaxed);
            delete node;
        }
    }

    void push(T value) {
        Node<T>* newNode = new Node<T>(value);  // 创建新节点
        newNode->next = head.load(std::memory_order_relaxed);  // 新节点指向当前栈顶

        // 原子地更新栈顶，直到成功
        while (!head.compare_exchange_weak(newNode->next, newNode,
                                           std::memory_order_release,
                                           std::memory_order_relaxed));
    }

    bool pop(T& value) {
        Node<T>* oldHead = head.load(std::memory_order_relaxed);

        // 试图更新栈顶，如果栈非空则出栈
        while (oldHead && !head.compare_exchange_weak(oldHead, oldHead->next,
                                                      std::memory_order_release,
                                                      std::memory_order_relaxed));

        if (oldHead == nullptr) {
            return false;  // 栈空，无法出栈
        }

        value = oldHead->value;  // 读取栈顶节点值
        delete oldHead;  // 释放栈顶节点
        return true;
    }
};

int main() {
    LockFreeStack<int> stack;

    stack.push(1);
    stack.push(2);
    stack.push(3);

    int value;
    while (stack.pop(value)) {
        std::cout << "Popped: " << value << std::endl;
    }

    return 0;
}
