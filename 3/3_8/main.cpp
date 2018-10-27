#include <iostream>
#include <map>
#include <mutex>
#include <boost/thread/shared_mutex.hpp>
#include <chrono>
#include <thread>
using namespace std;

class sharedVal
{
    int val{0};
    mutable  boost::shared_mutex entry_mutex;
public:
    int read_val() const
    {
        boost::shared_lock<boost::shared_mutex> lk(entry_mutex);
        return val;
    }

    void set_val(int & v)
    {
        std::lock_guard<boost::shared_mutex> lk(entry_mutex);
        val = v;
    }
};

void f1(sharedVal & dC)
{
    while(dC.read_val() != 10)
    {
        cout << "f1 val != 10 : " << dC.read_val() << endl;
    }
    cout << "f1 val = 10 : " << dC.read_val() << endl;
}

void f2(sharedVal & dC)
{
    while(dC.read_val() != 10)
    {
        cout << "f2 val != 10 : " << dC.read_val() << endl;
    }
    cout << "f2 val = 10 : " << dC.read_val() << endl;
}

void f3(sharedVal & dC)
{
    for(auto i : {5, 6, 7, 8, 9, 10})
    {
        dC.set_val(i);
        this_thread::sleep_for(1s);
    }
}

int main()
{
    sharedVal dC;

    thread t1(f1, ref(dC));
    thread t2(f2, ref(dC));
    thread t3(f3, ref(dC));
    t1.join();
    t2.join();
    t3.join();
    return 0;
}

