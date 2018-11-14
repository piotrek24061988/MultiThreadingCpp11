#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
using namespace std;

atomic<bool> x, y, z;

void write_x_then_y() {
    x.store(true, memory_order_relaxed);
    atomic_thread_fence(memory_order_release);
    y.store(true, memory_order_relaxed);
}

void read_y_then_x() {
    while(!y.load(memory_order_relaxed));
    atomic_thread_fence(memory_order_acquire);
    if(x.load(memory_order_relaxed)) {
        z.store(true, memory_order_relaxed);
    }
}

int main() {
    x = y = z = false;

    thread a(write_x_then_y);
    thread b(read_y_then_x);

    a.join(); b.join();

    cout << "z = 1: " << z.load() << endl;
}
