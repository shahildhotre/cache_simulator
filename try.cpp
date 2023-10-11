#include <iostream>

struct MyStruct {
    int a;
    char b;
};

void funcByValue(MyStruct s) {
    // Modify the copy of the struct
    s.a = 100;
    s.b = 'X';
}

int main() {
    MyStruct original = {10, 'A'};

    std::cout << "Original struct: a = " << original.a << ", b = " << original.b << std::endl;

    funcByValue(original);

    std::cout << "After calling funcByValue: a = " << original.a << ", b = " << original.b << std::endl;

    return 0;
}

