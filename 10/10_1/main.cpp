#include <iostream>
#include <algorithm>
#include <functional>
#include <numeric>
#include <thread>
#include <vector>
#include <list>
#include <future>
#include <exception>
#include <numeric>
#include <mutex>
#include <atomic>
#include <chrono>
#include <exception>
#include <queue>
#include <condition_variable>
#include <cassert>
using namespace std;

template<typename T>
class threadsafe_queue
{
private:
    mutable mutex mut;
    queue<shared_ptr<T>> data_queue;
    condition_variable data_cond;
public:
    threadsafe_queue(){}

    void push(T new_value)
    {
        shared_ptr<T> data(make_shared<T>(move(new_value)));
        lock_guard<mutex> lk(mut);
        data_queue.push(data);
        data_cond.notify_one();
    }

    void wait_and_pop(T & value)
    {
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [this](){return !data_queue.empty();});
        value = move(*data_queue.front());
        data_queue.pop();
    }

    shared_ptr<T> wait_and_pop()
    {
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [this](){return !data_queue.empty();});
        shared_ptr<T> res = data_queue.front();
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
        value = move(*data_queue.front());
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
        shared_ptr<T> res = data_queue.front();
        data_queue.pop();
        return res;
    }

    bool empty() const
    {
        lock_guard<mutex> lk(mut);
        return data_queue.empty();
    }
};

void test_concurrent_push_and_pop_on_empty_queue()
{
    threadsafe_queue<int> q;

    promise<void> go, push_ready, pop_ready;
    shared_future<void> ready(go.get_future());

    future<void> push_done;
    future<shared_ptr<int>> pop_done;

    try
    {
        pop_done = async(launch::async, [&q, ready, &pop_ready](){pop_ready.set_value();ready.wait(); return q.wait_and_pop();});
        push_done = async(launch::async, [&q, ready, &push_ready](){push_ready.set_value();ready.wait();q.push(42);});

        push_ready.get_future().wait();
        pop_ready.get_future().wait();
        go.set_value();

        push_done.get();
        assert(*(pop_done.get().get()) == 42);
        assert(q.empty());
    }
    catch(...)
    {
        go.set_value();
        throw;
    }
}

int main()
{
    test_concurrent_push_and_pop_on_empty_queue();
    return 0;
}

