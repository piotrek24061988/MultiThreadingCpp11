#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
using namespace std;

atomic<bool> x, y;
atomic<int> z;

void write_x() {
    x.store(true, memory_order_seq_cst);
}

void write_y() {
    y.store(true, memory_order_seq_cst);
}

void read_x_then_y() {
    while(!x.load(memory_order_seq_cst));
    if(y.load(memory_order_seq_cst)) {
        z++;
    }
}

void read_y_then_x() {
    while(!y.load(memory_order_seq_cst));
    if(x.load(memory_order_seq_cst)) {
        z++;
    }
}

int main() {
    x = y = false;
    z = 0;

    thread a(write_x);
    thread b(write_y);
    thread c(read_x_then_y); //One of these function
    thread d(read_y_then_x); //increments z for sure.

    a.join(); b.join(); c.join(); d.join();

    cout << "z = 1 or 2 z: " << z.load(memory_order_seq_cst) << endl;
}

