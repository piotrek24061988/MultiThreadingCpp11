#include <iostream>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <queue>
#include <thread>
using namespace std;

struct data_to_process
{
    mutex mut;
    queue<int> data_queue;
    condition_variable data_cond;
};

void data_preparation_thread(data_to_process & data)
{
    for(int i = 0; i < 10; i++)//more data to prepare
    {
        this_thread::sleep_for(1s);
        lock_guard<mutex> lk(data.mut);
        data.data_queue.push(i);
        data.data_cond.notify_one();
    }
}

void data_process_thread1(data_to_process & data)
{
    while(true)
    {
        unique_lock<mutex> lk(data. mut);
        data.data_cond.wait(lk, [&data](){return !data.data_queue.empty();});
        int c = data.data_queue.front();
        data.data_queue.pop();
        cout << "data_proces_thread1 var: " << c << endl;
    }
}

void data_process_thread2(data_to_process & data)
{
    while(true)
    {
        unique_lock<mutex> lk(data. mut);
        data.data_cond.wait(lk, [&data](){return !data.data_queue.empty();});
        int c = data.data_queue.front();
        data.data_queue.pop();
        cout << "data_proces_thread2 var: " << c << endl;
    }
}

int main()
{
    data_to_process data;
    thread t1(data_preparation_thread, ref(data));
    thread t2(data_process_thread1, ref(data));
    thread t3(data_process_thread2, ref(data));
    t1.join();
    t2.detach();
    t3.detach();
    return 0;
}

