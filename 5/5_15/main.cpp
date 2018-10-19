#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
using namespace std;

bool x;
atomic<bool> y;
atomic<int> z;

void write_x_then_y()
{
    x = true;
    atomic_thread_fence(memory_order_release);
    y.store(true, memory_order_relaxed);
}

void read_y_then_x()
{
    while(!y.load(memory_order_relaxed));
    atomic_thread_fence(memory_order_acquire);
    if(x)
    {
        z++;
    }
}

int main()
{
    x = false;
    y = false;
    z = 0;

    thread a(write_x_then_y);
    thread b(read_y_then_x);
    a.join();
    b.join();
    cout << "z = 1: " << z.load() << endl;

    return 0;
}
