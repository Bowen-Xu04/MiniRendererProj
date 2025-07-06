#include<cstdio>
#include<assert.h>

class A {
public:
    virtual void print() {
        printf("A\n");
    }
};

class B : public A {
public:
    virtual void print() override {
        printf("B\n");
    }
};

class C : public B {
public:
    virtual void print() override {
        printf("C\n");
    }
};

int main() {
    assert(0);
    A* a = new C;
    a->print();
    return 0;
}
