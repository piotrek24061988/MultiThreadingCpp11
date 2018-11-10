#include <iostream>
#include <atomic>
#include <thread>
using namespace std;

atomic<bool> x, y, z;

void write_x_then_y() {
    x.store(true, memory_order_relaxed);
    y.store(true, memory_order_relaxed);
}

void read_y_then_x()
{
    while(!y.load(memory_order_relaxed));
    if(x.load(memory_order_relaxed)) {
       z.store(true, memory_order_relaxed);
    }
}

int main() {
    x = y = z = false;

    thread a(write_x_then_y);
    thread b(read_y_then_x);
    a.join(); b.join();

    cout << "z can be 1 or 0, z = " << z.load(memory_order_relaxed) << endl;
}
