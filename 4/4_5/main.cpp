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

int f2()
{
    cout << "f2" << endl;
    this_thread::sleep_for(1s);
    return 2;
}

int f3()
{
    cout << "f3" << endl;
    this_thread::sleep_for(1s);
    return 3;
}

mutex m;
vector<packaged_task<int()>> pack_tasks;
vector<function<int()>> tasks = {f1, f2, f3};

template <typename Func>
future<int> post_task(Func f)
{
    packaged_task<int()> task(f);
    future<int> res = task.get_future();
    lock_guard<mutex> lk(m);
    pack_tasks.push_back(move(task));
    return res;
}

void push_thread()
{
    for(auto & task : tasks)
    {
        auto fut = post_task(task);
        cout << fut.get() << endl;
        this_thread::sleep_for(1s);
    }
}

void exec_task_thread()
{
    for(int i = 0; i < tasks.size();)
    {
        packaged_task<int()> task;
        {
            this_thread::sleep_for(1s);
            lock_guard<mutex> lk(m);
            if(!pack_tasks.size())
            {
                continue;
            }
            else
            {
                i++;
                task = move(pack_tasks.front());
                pack_tasks.pop_back();
            }
        }
        task();
    }
}

int main()
{
    thread t1(push_thread);
    thread t2(exec_task_thread);
    t1.join();
    t2.join();
    return 0;
}

