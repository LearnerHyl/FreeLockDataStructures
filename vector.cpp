#include <iostream>
#include <stdexcept>

template<typename T>
class SimpleVector {
private:
    T* data;          // 指向动态分配数组的指针
    size_t capacity;  // 容量
    size_t length;    // 当前长度

    void expandCapacity() {
        if (capacity == 0) capacity = 1;
        else capacity *= 2;
        T* newData = new T[capacity];
        for (size_t i = 0; i < length; i++) {
            newData[i] = data[i];
        }
        delete[] data;
        data = newData;
    }

public:
    SimpleVector() : data(nullptr), capacity(0), length(0) {}
    ~SimpleVector() {
        delete[] data;
    }

    void push_back(const T& value) {
        if (length >= capacity) {
            expandCapacity();
        }
        data[length++] = value;
    }

    T& operator[](size_t index) {
        if (index >= length) throw std::out_of_range("Index out of range");
        return data[index];
    }

    size_t size() const {
        return length;
    }

    // 提供对 const 对象的访问
    const T& operator[](size_t index) const {
        if (index >= length) throw std::out_of_range("Index out of range");
        return data[index];
    }
};

int main() {
    SimpleVector<int> vec;
    vec.push_back(10);
    vec.push_back(20);
    vec.push_back(30);

    std::cout << "Vector elements: ";
    for (size_t i = 0; i < vec.size(); i++) {
        std::cout << vec[i] << " ";
    }
    std::cout << std::endl;

    return 0;
}
