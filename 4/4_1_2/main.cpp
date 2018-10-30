#include <iostream>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <thread>
using namespace std;

mutex mut;
condition_variable data_cond;
int val;

void data_preparation_thread()
{
    for(int i = 1; i < 10; i++) {
        this_thread::sleep_for(1s);
        lock_guard<mutex> lk(mut);
        val = i;
        data_cond.notify_one();
    }
}

void data_process_thread()
{
    for(int i = 1; i < 10; i++) {
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [](){return !!val;});
        cout << "data_proces_thread1 var: " << val << endl;
        val = 0;
    }
}

int main()
{
    thread t1(data_preparation_thread);
    thread t2(data_process_thread);
    t1.join();
    t2.join();
    return 0;
}

