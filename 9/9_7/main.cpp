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

void interruption_point();

class interrupt_flag
{
    atomic<bool> flag;
    condition_variable * thread_cond;
    condition_variable_any * thread_cond_any;
    mutex set_clear_mutex;

public:
    interrupt_flag() : thread_cond(nullptr), thread_cond_any(nullptr) {}

    void set() {
        flag.store(true, memory_order_relaxed);
        lock_guard<mutex> lk(set_clear_mutex);
        if(thread_cond) {
            thread_cond->notify_all();
        } else if(thread_cond_any) {
            thread_cond_any->notify_all();
        }
    }

    template<typename Lockable>
    void wait(condition_variable_any & cv, Lockable & lk) {
        struct custom_lock
        {
            interrupt_flag * self;
            Lockable & lk;

            custom_lock(interrupt_flag * self_, condition_variable_any & cond, Lockable & lk_) : self(self_), lk(lk_) {
                self->set_clear_mutex.lock();
                self->thread_cond_any=&cond;
            }

            void unlock() {
                lk.unlock();
                self->set_clear_mutex.unlock();
            }

            void lock() {
                std::lock(self->set_clear_mutex, lk);
            }

            ~custom_lock() {
                self->thread_cond_any = 0;
                self->set_clear_mutex.unlock();
            }
        };

        custom_lock cl(this, cv, lk);
        interruption_point();
        cv.wait(cl);
        interruption_point();
    }

    bool is_set() const {
        return flag.load(memory_order_relaxed);
    }

    void set_condition_variable(condition_variable & cv) {
        lock_guard<mutex> lk(set_clear_mutex);
        thread_cond = &cv;
    }

    void clear_condition_variable() {
        lock_guard<mutex> lk(set_clear_mutex);
        thread_cond = 0;
    }
};
thread_local interrupt_flag this_thread_interrupt_flag;

class thread_interrupted {};

//To be colled in function done in separated thread.
//Mark a point when this function execution can be innterupted.
void interruption_point() {
    if(this_thread_interrupt_flag.is_set()) {
        throw thread_interrupted();
    }
}

template<typename Lockable>
void interruptible_wait(condition_variable_any & cv, Lockable &lk) {
    this_thread_interrupt_flag.wait(cv, lk);
}

template<typename T>
void interruptible_wait(future<T>& uf)
{
    while(!this_thread_interrupt_flag.is_set()) {
        if(uf.wait_for(chrono::microseconds(1)) == future_status::ready) {
            break;
        }
        interruption_point();
    }
}

//Thread which work can be interrupted.
struct interruptible_thread
{
    thread internal_thread;
    interrupt_flag * flag;

public:
    template <typename FunctionType>
    interruptible_thread(FunctionType f) {
        promise<interrupt_flag*>p;       //Initialize this_thread_interupt flag and run task.
        internal_thread=thread([f, &p](){
            p.set_value(&this_thread_interrupt_flag);
            try{
                f();
            }catch(thread_interrupted const &){
                cout << "thread_id: " << this_thread::get_id() << " interrupted" <<endl;
            }
        });
        flag = p.get_future().get();
    }

    void join() {
        internal_thread.join();
    }

    void detach() {
        internal_thread.detach();
    }

    bool joinable() const {
        return internal_thread.joinable();
    }

    void interrupt() {//interrupt thread by calling srt function pn this_thread_interrupt_flag.
        if(flag) {
            flag->set();
        }
    }
};

void foo1() {
    for(int i = 0; i < 20; i++) {
        if(i >= 10) interruption_point();
        cout << "thread_id: " << this_thread::get_id() << ", i = " << i << endl;
        this_thread::sleep_for(1s);
    }
}

void foo2() {
    cout << "thread_id: " << this_thread::get_id() << " interruptible_wait1" << endl;
    mutex lk;
    condition_variable_any cv;
    interruptible_wait(cv, lk);
}

void foo3() {
    cout << "thread_id: " << this_thread::get_id() << " interruptible_wait2" << endl;
    promise<int> pr;
    future<int> fut = pr.get_future();
    interruptible_wait(fut);
}

int main()
{
    //Thread finalized because of work done.
    vector<interruptible_thread> threads1;
    threads1.push_back(interruptible_thread(foo1));
    threads1.push_back(interruptible_thread(foo1));

    for(auto & t : threads1) {
        t.join();
    }

    //Threads finalized because of interruption.
    vector<interruptible_thread> threads2;
    threads2.push_back(interruptible_thread(foo1));
    threads2.push_back(interruptible_thread(foo1));

    this_thread::sleep_for(5s);

    for(auto & t: threads2) {
        t.interrupt();
    }
    for(auto & t : threads2) {
        t.join();
    }

    //Threads finalized because of interruption
    //during waiting.
    vector<interruptible_thread> threads3;
    threads3.push_back(interruptible_thread(foo2));
    threads3.push_back(interruptible_thread(foo2));

    this_thread::sleep_for(5s);

    for(auto & t: threads3) {
        t.interrupt();
    }
    for(auto & t : threads3) {
        t.join();
    }

    //Threads finalized because of interruption
    //during waiting2.
    vector<interruptible_thread> threads4;
    threads4.push_back(interruptible_thread(foo3));
    threads4.push_back(interruptible_thread(foo3));

    this_thread::sleep_for(5s);

    for(auto & t: threads4) {
        t.interrupt();
    }
    for(auto & t : threads4) {
        t.join();
    }
}
