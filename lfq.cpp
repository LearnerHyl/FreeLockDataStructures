#include <atomic>
#include <iostream>

template <typename T>
class Node
{
    T val;
    std::atomic<Node<T> *> next;

    Node(T value) : val(value), next(nullptr)
    {
    }
    Node() : next(nullptr) {}
};

template <typename T>
class LockFreeQueue
{
private:
    std::atomic<Node<T> *> head;
    std::atomic<Node<T> *> tail;

public:
    LockFreeQueue()
    {
        Node<T> *dummyNd = new Node<T>(-1);
        head.store(dummyNd, std::memory_order_relaxed);
        tail.store(dummyNd, std::memory_order_relaxed);
    }

    ~LockFreeQueue()
    {
        // 一般来说，清空队列操作只执行依次，因此不需要多线程之间的重排
        while (Node<T> *nd = head.load(std::memory_order_relaxed))
        {
            head.store(nd->next.load(std::memory_order_relaxed), std::memory_order_relaxed);
            delete nd;
        }
    }

    void enqueue(T value)
    {
        Node<T> *nd = new Node<T>(value);
        Node<T> *oldTail = tail.load(std::memory_order_relaxed);
        Node<T> *nullNode = nullptr;
        while (true)
        {
            oldTail = tail.load(std::memory_order_relaxed);
            // 确保在多线程场景下能看到别的线程的enqueue操作
            Node<T> *next = oldTail->next.load(std::memory_order_acquire);
            // tail没变，可以进行插入操作
            if (oldTail == tail.load(std::memory_order_acquire))
            {
                if (next == nullptr)
                {
                    // 若成功插入节点
                    if (oldTail->next.compare_exchange_weak(nullNode, nd, std::memory_order_release, std::memory_order_relaxed))
                    {
                        tail.compare_exchange_strong(oldTail, nd, std::memory_order_release, std::memory_order_relaxed);
                        break;
                    }
                }
                else
                { // 如果尾部next不为空，说明尾部已经被其他线程更新，尝试推进尾部指针
                    tail.compare_exchange_strong(oldTail, next, std::memory_order_release, std::memory_order_relaxed);
                }
            }
        }
    }

    bool dequeue(T &value)
    {
        Node<T> *oldHead;
        Node<T> *oldTail;
        Node<T> *next;

        while (true)
        {
            oldHead = head.load(std::memory_order_relaxed);
            oldTail = head.load(std::memory_order_relaxed);
            next = head->next.load(std::memory_order_acquire);

            if (oldHead == head.load(std::memory_order_acquire))
            {
                if (oldHead == oldTail)
                {
                    // 队列为空
                    if (next == nullptr)
                    {
                        return false;
                    }
                    // 帮助队列完成第一次入队操作
                    tail.compare_exchange_strong(oldTail, next, std::memory_order_release, std::memory_order_relaxed); // 尝试推进尾部指针
                }
                else
                {
                    value = next->val;
                    if (head.compare_exchange_strong(oldHead, next, std::memory_order_release, std::memory_order_relaxed))
                    {
                        break;
                    }
                }
            }
        }
        delete oldHead return true;
    }
};