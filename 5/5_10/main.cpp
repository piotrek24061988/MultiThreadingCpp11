#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
using namespace std;

atomic<bool> x, y;
atomic<int> z;

void write_x_then_y()
{
    x.store(true, memory_order_relaxed);
    y.store(true, memory_order_release);
}

void read_y_then_x()
{
    while(!y.load(memory_order_acquire));
    if(x.load(memory_order_relaxed))
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
