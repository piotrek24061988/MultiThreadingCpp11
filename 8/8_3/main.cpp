#include <iostream>
#include <thread>
#include <atomic>
using namespace std;

void processing_loop(atomic<unsigned long long int> & counter, const int & i) {
    while(counter.fetch_add(1, memory_order_relaxed) < 1000) {
        cout << "counter_" << i << ": " << counter << endl;
    }
}

int main()
{
    atomic<unsigned long long int> counter{0};

    thread t1(processing_loop, ref(counter), 1);
    thread t2(processing_loop, ref(counter), 2);
    thread t3(processing_loop, ref(counter), 3);

    t1.join(); t2.join(); t3.join();
}

