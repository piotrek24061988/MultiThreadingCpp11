#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
using namespace std;

struct X {
    int i;
    string s;
};

atomic<X*> p;
atomic<int> a;

void create_x()
{
    X * x = new X;
    x->i = 42;
    x->s = "witaj";
    a.store(99, memory_order_relaxed);
    p.store(x, memory_order_release);
}

void use_x()
{
    X * x;
    while(!(x = p.load(memory_order_consume)))
        this_thread::sleep_for(chrono::microseconds(1));

    cout << (x->i == 42) << endl;
    cout << (x->s == "witaj") << endl;
    cout << (a.load(memory_order_relaxed) == 99) << endl;
}

int main()
{
    thread t1(create_x);
    thread t2(use_x);

    t2.join();
    t1.join();

    return 0;
}

