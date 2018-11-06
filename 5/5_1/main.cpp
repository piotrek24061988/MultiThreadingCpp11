#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
using namespace std;

//Basic atomic type atomic_flag used to implement spin lock
class spinlock_mutex {
    atomic_flag flag; //Basic atomic type with only 2 operations: clear - write (to false).
                      //test_and_set - read current value. Modify (to true). Write.
public:
    spinlock_mutex() : flag(ATOMIC_FLAG_INIT) {}

    void lock() {
        while(flag.test_and_set(memory_order_acquire)) { //Read, modification, write operation.
            cout << "already locked, waiting" << endl;   //Write to true.
            this_thread::sleep_for(1s);
        }
        cout << "already unlocked, locking"<< endl;
    }

    void unlock() {
        flag.clear(memory_order_acquire);//Write operation. Write to false.
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
    this_thread::sleep_for(2s);
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
}

