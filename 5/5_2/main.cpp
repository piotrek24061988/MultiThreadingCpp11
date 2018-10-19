#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
using namespace std;

class spinlock_mutex
{
    atomic<bool> flag;

public:
    spinlock_mutex()
    {
        flag.store(false);
        cout << "is lock free: " << flag.is_lock_free() << endl;
    }

    void lock()
    {
        while(flag.exchange(true, memory_order_acquire))
        {
            //only for debug purpose
            cout << "already locked, waiting" << endl;
            this_thread::sleep_for(1s);
        }
            //only for debug purpose
            cout << "already unlocked, locking"<< endl;
    }

    void unlock()
    {
        flag.store(false);
    }

    bool try_lock()
    {
        return !flag.exchange(true, memory_order_acquire);
    }
};

void f1(spinlock_mutex & slm)
{
    cout << "f1 begining" << endl;
    this_thread::sleep_for(2s);
    slm.lock();
    cout << "f1 slm locked" << endl;
    this_thread::sleep_for(5s);
    slm.unlock();
    cout << "f1 slm unlocked" << endl;
}

void f2(spinlock_mutex & slm)
{
    cout << "f2 begining" << endl;
    this_thread::sleep_for(2s);
    slm.lock();
    cout << "f2 slm locked" << endl;
    this_thread::sleep_for(5s);
    slm.unlock();
    cout << "f2 slm unlocked" << endl;
}

void f3(spinlock_mutex & slm)
{
    cout << "f3 begining" << endl;
    this_thread::sleep_for(3s);
    while(!slm.try_lock())
    {
       cout << "f3 slm lock unseccesfull" << endl;
       this_thread::sleep_for(1s);
    }
    cout << "f3 slm locked" << endl;
    this_thread::sleep_for(2s);
    slm.unlock();
    cout << "f3 slm unlocked" << endl;
}

int main()
{
    spinlock_mutex slm;

    thread t1(f1, ref(slm));
    this_thread::sleep_for(1s);
    thread t2(f2, ref(slm));
    this_thread::sleep_for(1s);
    thread t3(f3, ref(slm));
    t1.join();
    t2.join();
    t3.join();
    return 0;
}

