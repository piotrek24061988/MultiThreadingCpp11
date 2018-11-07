#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
using namespace std;

//Atomic type atomic<bool> used to implement spin lock
class spinlock_mutex {
    atomic<bool> flag;

public:
    spinlock_mutex() {
        flag.store(false);//Write operation.
        //Is this type in my implementation without internal blockads?
        cout << "is lock free: " << flag.is_lock_free() << endl;
    }

    void lock() {
        //Read, modification, write operation.
        while(flag.exchange(true, memory_order_acquire)) {
            cout << "already locked, waiting" << endl;
            this_thread::sleep_for(1s);
        }
        cout << "already unlocked, locking"<< endl;
    }

    void unlock() {
        flag.store(false);//Write operation.
    }

    bool try_lock() {
        //Read, modification, write operation.
        return !flag.exchange(true, memory_order_acquire);
    }
};

void f1(spinlock_mutex & slm) {
    cout << "f1 begining" << endl;
    this_thread::sleep_for(2s);
    slm.lock();
    cout << "f1 slm locked" << endl;
    this_thread::sleep_for(5s);
    slm.unlock();
    cout << "f1 slm unlocked" << endl;
}

void f2(spinlock_mutex & slm) {
    cout << "f2 begining" << endl;
    this_thread::sleep_for(2s);
    slm.lock();
    cout << "f2 slm locked" << endl;
    this_thread::sleep_for(5s);
    slm.unlock();
    cout << "f2 slm unlocked" << endl;
}

int main()
{
    spinlock_mutex slm;

    thread t1(f1, ref(slm));
    this_thread::sleep_for(1s);
    thread t2(f2, ref(slm));
    t1.join();
    t2.join();
    return 0;
}

