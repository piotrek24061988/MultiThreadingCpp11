#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
using namespace std;

atomic<bool> x, y, z;

void write_x_then_y() {
    x.store(true, memory_order_relaxed);
    y.store(true, memory_order_release);
}

void read_y_then_x() {
    while(!y.load(memory_order_acquire));
    if(x.load(memory_order_relaxed)) {
        z.store(true, memory_order_release);
    }
}

int main() {
    x = y = z = false;

    thread a(write_x_then_y);
    thread b(read_y_then_x);
    a.join();
    b.join();
    cout << "z = always 1: " << z.load(memory_order_acquire) << endl;

    return 0;
}
