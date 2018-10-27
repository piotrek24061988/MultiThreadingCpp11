#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
using namespace std;

class CommSlot
{
    once_flag of;

    void open(const string & s)
    {
        cout << "open() " << s << endl;
    }
public:
    void send()
    {
        call_once(of, &CommSlot::open, this, "from send()");
        cout << "send()" << endl;
    }
    void receive()
    {
        call_once(of, &CommSlot::open, this, "from receive");
        cout << "receive()" << endl;
    }

    static CommSlot & getInstance()
    {
        static CommSlot Cs;
        return Cs;
    }

};

void f1()
{
    for(int i = 0; i < 5; i++)
    {
        CommSlot::getInstance().send();
        this_thread::sleep_for(1s);
    }
}

void f2()
{
    for(int i = 0; i < 5; i++)
    {
        CommSlot::getInstance().receive();
        this_thread::sleep_for(1s);
    }
}

int main()
{
    thread t1(f1);
    thread t2(f2);
    t1.join();
    t2.join();
    return 0;
}

