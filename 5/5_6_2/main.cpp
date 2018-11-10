#include <iostream>
#include <atomic>
#include <thread>
using namespace std;

atomic<bool> x, y, z;

void write_x_then_y() {
    x.store(true, memory_order_seq_cst);
    y.store(true, memory_order_seq_cst);
}

void read_y_then_x()
{
    while(!y.load(memory_order_seq_cst));
    if(x.load(memory_order_seq_cst)) {
       z.store(true, memory_order_seq_cst);
    }
}

int main() {
    x = y = z = false;

    thread a(write_x_then_y);
    thread b(read_y_then_x);
    a.join(); b.join();

    cout << "z must be 1, z = " << z.load(memory_order_seq_cst) << endl;
}
