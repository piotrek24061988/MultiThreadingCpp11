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
    interrupt_flag()
        : thread_cond(nullptr), thread_cond_any(nullptr)
    {}

    void set()
    {
        flag.store(true, memory_order_relaxed);
        lock_guard<mutex> lk(set_clear_mutex);
        if(thread_cond)
        {
            thread_cond->notify_all();
        }
        else if(thread_cond_any)
        {
            thread_cond_any->notify_all();
        }
    }

    template<typename Lockable>
    void wait(condition_variable_any & cv, Lockable & lk)
    {
        struct custom_lock
        {
            interrupt_flag * self;
            Lockable & lk;

            custom_lock(interrupt_flag * self_, condition_variable_any & cond, Lockable & lk_) : self(self_), lk(lk_)
            {
                self->set_clear_mutex.lock();
                self->thread_cond_any=&cond;
            }

            void unlock()
            {
                lk.unlock();
                self->set_clear_mutex.unlock();
            }

            void lock()
            {
                lock(self->set_clear_mutex, lk);
            }

            ~custom_lock()
            {
                self->thread_cond_any = 0;
                self->set_clear_mutex.unlock();
            }
        };
        custom_lock cl(this, cv, lk);
        interruption_point();
        cv.wait(cl);
        interruption_point();
    }

    bool is_set() const
    {
        return flag.load(memory_order_relaxed);
    }

    void set_condition_variable(condition_variable & cv)
    {
        lock_guard<mutex> lk(set_clear_mutex);
        thread_cond = &cv;
    }

    void clear_condition_variable()
    {
        lock_guard<mutex> lk(set_clear_mutex);
        thread_cond = 0;
    }
};

thread_local interrupt_flag this_thread_interrupt_flag;

class thread_interrupted
{
};


void interruption_point()
{
    if(this_thread_interrupt_flag.is_set())
    {
        throw thread_interrupted();
    }
}

struct clear_cv_on_destruct
{
    ~clear_cv_on_destruct()
    {
        this_thread_interrupt_flag.clear_condition_variable();
    }
};

template<typename Lockable>
void interruptible_wait(condition_variable & cv, Lockable &lk)
{
    this_thread_interrupt_flag.wait(cv, lk);
}

template<typename T, typename Lockable>
void interruptible_wait(future<T>& uf, Lockable &lk)
{
    while(!this_thread_interrupt_flag.is_set())
    {
        if(uf.wait_for(lk, chrono::microseconds(1)) == future_status::ready)
        {
            break;
        }
        interruption_point();
    }
}

struct interruptible_thread
{
    thread internal_thread;
    interrupt_flag * flag;

public:
    template <typename FunctionType>
    interruptible_thread(FunctionType f)
    {
        promise<interrupt_flag*>p;
        internal_thread=thread([f, &p](){p.set_value(&this_thread_interrupt_flag);try{f();}catch(...){}});
        flag = p.get_future().get();
    }

    void join()
    {
        internal_thread.join();
    }

    void detach();
    bool joinable() const;
    void interrupt()
    {
        if(flag)
        {
            flag->set();
        }
    }
};

void f1()
{
    while(true)
    {
        interruption_point();
        cout << "f1()" << endl;
        this_thread::sleep_for(1s);
    }
}

void f2()
{
    while(true)
    {
        interruption_point();
        cout << "f2()" << endl;
        this_thread::sleep_for(1s);
    }
}

int main()
{
    vector<interruptible_thread> threads;
    threads.push_back(interruptible_thread(f1));
    threads.push_back(interruptible_thread(f2));

    this_thread::sleep_for(5s);

    for(auto & t: threads)
    {
        t.interrupt();
    }
    for(auto & t : threads)
    {
        t.join();
    }

    return 0;
}



