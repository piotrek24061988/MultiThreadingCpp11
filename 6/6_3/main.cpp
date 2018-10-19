#include <iostream>
#include <mutex>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <exception>
#include <stack>
#include <queue>
#include <condition_variable>
using namespace std;

struct empty_stack : exception
{
    const char * what() const throw()
    {

    }
};

template<typename T>
class threadsafe_queue
{
private:
    mutable mutex mut;
    queue<T> data_queue;
    condition_variable data_cond;
public:
    threadsafe_queue(){}

    void push(T new_value)
    {
        lock_guard<mutex> lk(mut);
        data_queue.push(move(new_value));
        data_cond.notify_one();
    }

    void wait_and_pop(T & value)
    {
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [this](){return !data_queue.empty();});
        value = move(data_queue.front());
        data_queue.pop();
    }

    shared_ptr<T> wait_and_pop()
    {
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [this](){return !data_queue.empty();});
        shared_ptr<T> res(make_shared<T>(move(data_queue.front())));
        data_queue.pop();
        return res;
    }

    bool try_pop(T & value)
    {
        lock_guard<mutex> lk(mut);
        if(data_queue.empty())
        {
            return false;
        }
        value = move(data_queue.front());
        data_queue.pop();
        return true;
    }

    shared_ptr<T> try_pop()
    {
        lock_guard<mutex> lk(mut);
        if(data_queue.empty())
        {
            return false;
        }
        shared_ptr<T> res(shared_ptr<T>(move(data_queue.front())));
        data_queue.pop();
        return res;
    }

    bool empty() const
    {
        lock_guard<mutex> lk(mut);
        return data_queue.empty();
    }
};

threadsafe_queue<int> tSq;
atomic<bool> endf;

void f1()
{
    endf = false;
    for(auto & a : {1, 2, 3, 4, 5, 6, 7,8 ,9})
    {
        tSq.push(a);
        this_thread::sleep_for(1s);
    }
    endf = true;
}

mutex m;

void f2()
{
    while(!endf)
    {
        cout << "f2: " << *(tSq.wait_and_pop()) << endl;
    }
}

void f3()
{
    while(!endf)
    {
        int val;
        tSq.wait_and_pop(val);
        cout << "f3: " << val << endl;
    }
}

void f4()
{
    while(!endf)
    {
        int val;
        tSq.wait_and_pop(val);
        cout << "f4: " << val << endl;
    }
}

int main()
{
   thread t2(f2);
   thread t3(f3);
   thread t4(f4);
   thread t1(f1);
   t2.detach();
   t3.detach();
   t4.detach();
   t1.join();
   return 0;
}

