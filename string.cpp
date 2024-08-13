#include <iostream>
#include <cstring>  // 使用 C-style string 函数，如 strlen 和 strcpy

class SimpleString {
private:
    char* data;
    size_t size;

    void allocate_and_copy(const char* str) {
        size = strlen(str);
        data = new char[size + 1];  // 加一为了 null-terminator
        strcpy(data, str);
    }

public:
    // 默认构造函数
    SimpleString() : data(nullptr), size(0) {
        data = new char[1];
        data[0] = '\0';
    }

    // 构造函数，从 C-string 初始化
    SimpleString(const char* str) {
        allocate_and_copy(str);
    }

    // 复制构造函数
    SimpleString(const SimpleString& other) {
        allocate_and_copy(other.data);
    }

    // 移动构造函数
    SimpleString(SimpleString&& other) noexcept : data(other.data), size(other.size) {
        other.data = nullptr;
        other.size = 0;
    }

    // 析构函数
    ~SimpleString() {
        delete[] data;
    }

    // 复制赋值操作符
    SimpleString& operator=(const SimpleString& other) {
        if (this == &other) return *this;
        delete[] data;
        allocate_and_copy(other.data);
        return *this;
    }

    // 移动赋值操作符
    SimpleString& operator=(SimpleString&& other) noexcept {
        if (this == &other) return *this;
        delete[] data;
        data = other.data;
        size = other.size;
        other.data = nullptr;
        other.size = 0;
        return *this;
    }

    // 获取字符串的长度
    size_t length() const {
        return size;
    }

    // C-style string 访问
    const char* c_str() const {
        return data;
    }
};

int main() {
    SimpleString str1("Hello");
    SimpleString str2 = str1;  // 测试复制构造函数

    std::cout << "str1: " << str1.c_str() << std::endl;
    std::cout << "str2: " << str2.c_str() << std::endl;

    SimpleString str3("World");
    str3 = std::move(str1);  // 测试移动赋值

    std::cout << "str3: " << str3.c_str() << std::endl;
    std::cout << "str1: " << (str1.c_str() ? str1.c_str() : "null") << std::endl;  // str1 应该是空的

    return 0;
}
