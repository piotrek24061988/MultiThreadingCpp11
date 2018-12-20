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

//Thread safe queue based on condition variable and std queue.
template<typename T>
class threadsafe_queue {
private:
    mutable mutex mut;
    queue<T> data_queue;
    condition_variable data_cond;
public:
    threadsafe_queue(){}

    void push(T new_value) {
        lock_guard<mutex> lk(mut);
        data_queue.push(move(new_value));
        data_cond.notify_one();
    }

    void wait_and_pop(T & value, int ms) {
        unique_lock<mutex> lk(mut);
        if(data_cond.wait_for(lk, chrono::milliseconds(ms), [this](){return !data_queue.empty();})) {
            value = move(data_queue.front());
            data_queue.pop();
        }
    }

    shared_ptr<T> wait_and_pop(int ms) {
        shared_ptr<T> res;
        unique_lock<mutex> lk(mut);
        if(data_cond.wait_for(lk, chrono::milliseconds(ms), [this](){return !data_queue.empty();})) {
            res = make_shared<T>(move(data_queue.front()));
            data_queue.pop();
        } else {
            res = make_shared<T>();
        }
        return res;
    }

    bool try_pop(T & value) {
        lock_guard<mutex> lk(mut);
        if(data_queue.empty()) {
            return false;
        }
        value = move(data_queue.front());
        data_queue.pop();
        return true;
    }

    shared_ptr<T> try_pop() {
        lock_guard<mutex> lk(mut);
        if(data_queue.empty()) {
            return shared_ptr<T>();
        }
        shared_ptr<T> res = make_shared<T>(move(data_queue.front()));
        data_queue.pop();
        return res;
    }

    bool empty() const {
        lock_guard<mutex> lk(mut);
        return data_queue.empty();
    }
};

//Push data to empty queue in one thread.
//WaitPop data from this queue in second thread.
//In the end pop data should be equal to pushed data.
//In the end queue should be empty.
void test_concurrent_push_and_waitpop_on_empty_queue()
{
    threadsafe_queue<int> q; //Tested queue.

    constexpr int test_val = 42;

    promise<void> go, push_ready, pop_ready;
    shared_future<void> ready(go.get_future());

    future<void> push_done;
    future<shared_ptr<int>> pop_done;

    try {
        //Thread geting value from threadsafe_queue.
        pop_done = async(launch::async, [&q, ready, &pop_ready](){
            pop_ready.set_value();//Notify main thread that poping thread is started.
            ready.wait();//Wait for main thread to start test.
            return q.wait_and_pop(1000);
        });
        //Thread inserting value to threadsafe_queue.
        push_done = async(launch::async, [&q, ready, &push_ready](){
            push_ready.set_value();//Notify main thread that poping thread is started.
            ready.wait();//Wait for main thread to start test.
            q.push(test_val);
        });

        push_ready.get_future().wait();//Wait for poping thread to start.
        pop_ready.get_future().wait();//Wait for pushing thread to start.
        go.set_value();//Start test.

        //When push and waiting pop done returned value should be equal to inserted value
        //and queue should be empty at the end.
        push_done.get();
        assert(*(pop_done.get().get()) == test_val);
        assert(              q.empty() == true);
        cout << "test_concurrent_push_and_waitpop_on_empty_queue: passed" << endl;
    } catch(...) {
        go.set_value();
        throw;
    }
}

//Push data to empty queue in one thread.
//TryPop data from this queue in second thread.
//In the end pop data should be empty and queue should not be empty or.
//In the end pop data should be equal to pushed data and queue should be empty.
void test_concurrent_push_and_trypop_on_empty_queue()
{
    threadsafe_queue<int> q; //Tested queue.

    constexpr int test_val = 42;

    promise<void> go, push_ready, pop_ready;
    shared_future<void> ready(go.get_future());

    future<void> push_done;
    future<shared_ptr<int>> pop_done;

    try {
        //Thread geting value from threadsafe_queue.
        pop_done = async(launch::async, [&q, ready, &pop_ready](){
            pop_ready.set_value();//Notify main thread that poping thread is started.
            ready.wait();//Wait for main thread to start test.
            return q.try_pop();
        });
        //Thread inserting value to threadsafe_queue.
        push_done = async(launch::async, [&q, ready, &push_ready](){
            push_ready.set_value();//Notify main thread that poping thread is started.
            ready.wait();//Wait for main thread to start test.
            q.push(test_val);
        });

        push_ready.get_future().wait();//Wait for poping thread to start.
        pop_ready.get_future().wait();//Wait for pushing thread to start.
        go.set_value();//Start test.

        //When push and trypop done.
        //There should be no returned value and queue should not be empty.
        //Or returned value should be equal to inserted value
        //and queue should be empty at the end.
        push_done.get();
        auto poped = pop_done.get().get();
        auto empty = q.empty();
        assert( ( ( poped == nullptr)  && !q.empty() ) ||
                ( (*poped == test_val) &&  q.empty() ) );
        cout << "test_concurrent_push_and_trypop_on_empty_queue: passed" << endl;
    } catch(...) {
        go.set_value();
        throw;
    }
}

int main() {
    test_concurrent_push_and_waitpop_on_empty_queue();
    test_concurrent_push_and_trypop_on_empty_queue();
}

