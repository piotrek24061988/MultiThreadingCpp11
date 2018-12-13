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
using namespace std;

template<typename T>
class threadsafe_queue {
private:
    mutable mutex mut;
    queue<shared_ptr<T>> data_queue;
    condition_variable data_cond;
public:
    threadsafe_queue(){}

    void push(T new_value) {
        shared_ptr<T> data = make_shared<T>(move(new_value));
        lock_guard<mutex> lk(mut);
        data_queue.push(data);
        data_cond.notify_one();
    }

    void wait_and_pop(T & value) {
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [this](){return !data_queue.empty();});
        value = move(*data_queue.front());
        data_queue.pop();
    }

    shared_ptr<T> wait_and_pop() {
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [this](){return !data_queue.empty();});
        shared_ptr<T> res = data_queue.front();
        data_queue.pop();
        return res;
    }

    bool try_pop(T & value) {
        lock_guard<mutex> lk(mut);
        if(data_queue.empty()) {
            return false;
        }
        value = move(*data_queue.front());
        data_queue.pop();
        return true;
    }

    shared_ptr<T> try_pop() {
        lock_guard<mutex> lk(mut);
        if(data_queue.empty()) {
            return shared_ptr<T>();
        }
        shared_ptr<T> res = data_queue.front();
        data_queue.pop();
        return res;
    }

    bool empty() const {
        lock_guard<mutex> lk(mut);
        return data_queue.empty();
    }
};

class join_threads
{
    vector<thread> & threads;
public:
    explicit join_threads(vector<thread> & threads_) : threads(threads_){}

    ~join_threads() {
        for(unsigned long i = 0; i < threads.size(); ++i) {
            if(threads[i].joinable()) {
                threads[i].join();
            }
        }
    }
};

class thread_pool
{
    atomic_bool done;
    threadsafe_queue<function<void()>> work_queue;
    vector<thread> threads;
    join_threads joiner;
    
    void worker_thread(unsigned i) {
        cout << "worker_thread: " << i << endl;
        while(!done) {
            function<void()> task;
            if(work_queue.try_pop(task)) {
                cout << "worker_thread: " << i << " doing task" << endl;
                task();
            } else {
                this_thread::yield();//tell scheduler to run other thread
            }
        }
    }

public:
    thread_pool() : done(false), joiner(threads) {
        unsigned const thread_count = thread::hardware_concurrency();
        try {
            for(unsigned i = 0; i < thread_count; i++) {
                threads.push_back(thread(&thread_pool::worker_thread, this, i));
            }
        } catch(...) {
            done = true;
            throw;
        }
    }

    ~thread_pool() {
        done = true;
    }

    template<typename FunctionType>
    void submit(FunctionType f) {
        work_queue.push(function<void()>(f));
    }
};

void f1() {
    cout << "f1" << endl;
    this_thread::sleep_for(1s);
}

void f2() {
    cout << "f2" << endl;
    this_thread::sleep_for(1s);
}

void f3() {
    cout << "f3" << endl;
    this_thread::sleep_for(1s);
}

int main()
{
    thread_pool tp;
    this_thread::sleep_for(5s);
    cout << "------------------------------" << endl;

    tp.submit(f1);
    this_thread::sleep_for(5s);
    cout << "------------------------------" << endl;

    tp.submit(f1);
    tp.submit(f2);
    this_thread::sleep_for(5s);
    cout << "------------------------------" << endl;

    tp.submit(f1);
    tp.submit(f2);
    tp.submit(f3);
    this_thread::sleep_for(5s);
    cout << "------------------------------" << endl;

    tp.submit(f1);
    tp.submit(f2);
    tp.submit(f3);
    tp.submit([](){cout << "f4" << endl; this_thread::sleep_for(1s);});
    this_thread::sleep_for(5s);
    cout << "------------------------------" << endl;
}

