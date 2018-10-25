#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <climits>
using namespace std;

class hierarchical_mutex
{
    mutex internal_mutex;
    unsigned long hierarchy_value;
    unsigned long previous_hierarchy_value;
    static /*thread_local*/ unsigned long this_thread_hierarchy_value;

    void check_for_hierarchy_violation()
    {
        if(this_thread_hierarchy_value <= hierarchy_value)
        {
            throw logic_error("naruszono hierarchie mutexow");
        }
    }

    void update_hierarchy_value()
    {
        previous_hierarchy_value = this_thread_hierarchy_value;
        this_thread_hierarchy_value = hierarchy_value;
    }
public:
    explicit hierarchical_mutex(unsigned long value)
        : hierarchy_value(value)
        , previous_hierarchy_value(0)
    {
    }

    void lock()
    {
        check_for_hierarchy_violation();
        internal_mutex.lock();
        update_hierarchy_value();
    }

    void unlock()
    {
        this_thread_hierarchy_value = previous_hierarchy_value;
        internal_mutex.unlock();
    }
    bool try_lock()
    {
        check_for_hierarchy_violation();
        if(!internal_mutex.try_lock())
        {
            return false;
        }
        update_hierarchy_value();
        return true;
    }
};

/*thread_local*/ unsigned long hierarchical_mutex::this_thread_hierarchy_value(ULONG_MAX);

hierarchical_mutex high_level_mutex(10000);
hierarchical_mutex low_level_mutex(5000);

void low_level_func()
{
    try
    {
       cout << "low_level_func" << endl;
       lock_guard<hierarchical_mutex> lk(low_level_mutex);
       this_thread::sleep_for(3s);
    }
    catch(...)
    {
        cout << "low_level_func exception ending" << endl;
        return;
    }
    cout << "low_level_func sucesfull ending" << endl;
}

void high_level_func()
{
    try
    {
        lock_guard<hierarchical_mutex> lk(high_level_mutex);
        cout << "high_level_func" << endl;
        this_thread::sleep_for(3s);
    }
    catch(...)
    {
        cout << "high_level_func exception ending" << endl;
        return;
    }
    cout << "high_level_func sucesfull ending" << endl;
}

int main()
{
    thread t1(high_level_func);
    this_thread::sleep_for(1s);
    thread t2(low_level_func);
    t1.join();
    t2.join();

    cout << endl << endl;
    this_thread::sleep_for(3s);

    thread t3(low_level_func);
    this_thread::sleep_for(1s);
    thread t4(high_level_func);
    t3.join();
    t4.join();

    return 0;
}


/*
int do_low_level_struff()
{
    cout << "do_low_level_struff" << endl;
    return 1;
}

int low_level_func()
{
    lock_guard<hierarchical_mutex> lk(low_level_mutex);
    return do_low_level_struff();
}

void high_level_stuff(int some_param)
{
    cout << "high_level_stuff: " << some_param << endl;
}

void high_level_func()
{
    lock_guard<hierarchical_mutex> lk(high_level_mutex);
    high_level_stuff(low_level_func());
}

hierarchical_mutex other_mutex(100);
void other_stuff()
{
    high_level_func();
}

void other_func()
{
    lock_guard<hierarchical_mutex> lk(other_mutex);
    try
    {
       other_stuff();
    }
    catch(logic_error & e)
    {
        cout << e.what() << endl;
    }
}

int main()
{
    thread t1(high_level_func);
    t1.join();

    thread t2(other_func);
    t2.join();

    return 0;
}

*/
