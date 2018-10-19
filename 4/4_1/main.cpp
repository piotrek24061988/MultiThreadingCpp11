#include <iostream>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <queue>
#include <thread>
using namespace std;

mutex mut;
queue<int> data_queue;
condition_variable data_cond;

void data_preparation_thread()
{
    for(int i = 0; i < 100; i++)//more data to prepare
    {
        this_thread::sleep_for(1s);
        lock_guard<mutex> lk(mut);
        data_queue.push(i);
        data_cond.notify_one();
    }
}

void data_process_thread()
{
    for(int i = 0; i < 100; i++)
    {
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [](){return !data_queue.empty();});
        int c = data_queue.front();
        data_queue.pop();
        lk.unlock();
        cout << "data_proces_thread var: " << c << endl;
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

