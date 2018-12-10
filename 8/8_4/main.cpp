#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
using namespace std;

bool processing_data(unsigned long long int & data, const int & i) {
    if(data <= 1000) {
        cout << "processing_data_" << i << ": " << data++ << endl;
        return false;
    }
    return true;
}

void processing_loop(mutex & m, unsigned long long int & data, const int & i) {
    while(true) {
        lock_guard<mutex> lock(m);
        if(processing_data(data, i)) break;
    }
}

int main()
{
    mutex m;
    unsigned long long int data {0};

    thread t1(processing_loop, ref(m) ,ref(data), 1);
    thread t2(processing_loop, ref(m) ,ref(data), 2);
    thread t3(processing_loop, ref(m) ,ref(data), 3);

    t1.join(); t2.join(); t3.join();
}
