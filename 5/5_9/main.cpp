#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
using namespace std;

atomic<bool> x, y, z;

void write_x() {
    x.store(true, memory_order_release);
}

void write_y() {
    y.store(true, memory_order_release);
}

void read_x_then_y() {
    while(!x.load(memory_order_acquire));
    if(y.load(memory_order_acquire)) {
        z.store(true, memory_order_release);
    }
}

void read_y_then_x() {
    while(!y.load(memory_order_acquire));
    if(x.load(memory_order_acquire)) {
        z.store(true, memory_order_release);
    }
}

int main() {
    x = y = z = false;

    thread a(write_x);
    thread b(write_y);
    thread c(read_x_then_y);
    thread d(read_y_then_x);

    a.join(); b.join(); c.join(); d.join();
    cout << "z = 0 or 1, z: " << z.load(memory_order_seq_cst) << endl;

    return 0;
}

