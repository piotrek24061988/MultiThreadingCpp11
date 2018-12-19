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

struct clear_cv_on_destruct {
    ~clear_cv_on_destruct() {
        this_thread_interrupt_flag.clear_condition_variable();
    }
};

class thread_interrupted {};

//To be colled in function done in separated thread.
//Mark a point when this function execution can be interrupted.
//Thread let know when it can be interrupted. So if someone called
//interrupt on interruptible_thread, it will be interrupted in first
//interrupt_point.
void interruption_point() {
    if(this_thread_interrupt_flag.is_set()) {
        throw thread_interrupted();
    }
}

template <typename Predicate>
void interruptible_wait(condition_variable & cv, std::unique_lock<mutex> &lk, Predicate pred) {
    interruption_point();
    this_thread_interrupt_flag.set_condition_variable(cv);
    clear_cv_on_destruct guard;
    while(!this_thread_interrupt_flag.is_set() && !pred()) {
        cv.wait_for(lk, chrono::microseconds(1));
    }
    interruption_point();
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

//Thread which work can be interrupted based on std::thread.
struct interruptible_thread
{
    thread internal_thread;
    interrupt_flag * flag;

public:
    template <typename FunctionType>
    interruptible_thread(FunctionType f) {
        promise<interrupt_flag*>p;       //Initialize this_thread interupt_flag and run task.
        internal_thread=thread([f, &p](){
            p.set_value(&this_thread_interrupt_flag);//Set future to this_thread_interrupt_flag.
            try{
                f();
            }catch(thread_interrupted const &){
                cout << "thread_id: " << this_thread::get_id() << " interrupted" <<endl;
            }
        });//Asign interrupt_flag to thread_local this_thread_interrupt_flag taken from future.
        flag = p.get_future().get();//Ctor is finishing work when thread is running.
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
    //Interrupt thread by calling set function on this_thread interrupt_flag.
    void interrupt() {
        if(flag) {
            flag->set();
        }
    }
};

void foo1() {
    for(int i = 0; i < 10; i++) {
        if(i >= 5) interruption_point();
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

void foo4() {
    cout << "thread_id: " << this_thread::get_id() << " interruptible_wait3" << endl;
    mutex m;
    unique_lock<mutex> lk(m);
    condition_variable cv;
    interruptible_wait(cv, lk, [](){return false;});
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

    this_thread::sleep_for(3s);

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

    this_thread::sleep_for(3s);

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

    this_thread::sleep_for(3s);

    for(auto & t: threads4) {
        t.interrupt();
    }
    for(auto & t : threads4) {
        t.join();
    }

    //Threads finalized because of interruption
    //during waiting3.
    vector<interruptible_thread> threads5;
    threads5.push_back(interruptible_thread(foo4));
    threads5.push_back(interruptible_thread(foo4));

    this_thread::sleep_for(3s);

    for(auto & t: threads5) {
        t.interrupt();
    }
    for(auto & t : threads5) {
        t.join();
    }
}
