#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <utility>
#include <future>
using namespace std;

int f1()
{
    cout << "f1" << endl;
    this_thread::sleep_for(1s);
    return 1;
}

void f2(packaged_task<int()> & task)
{
    cout << "f2" << endl;
    future<int> fut = task.get_future();
    cout << fut.get() << endl;
}

void f3(packaged_task<int()> & task)
{
    cout << "f3" << endl;
    task();
}

int main()
{
    packaged_task<int()> task(f1);
    thread t1(f2, ref(task));
    thread t2(f3, ref(task));
    t1.join();
    t2.join();
}

