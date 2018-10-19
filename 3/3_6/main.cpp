#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
using namespace std;

class CommSlot
{
    once_flag of;

    void open()
    {
        cout << "open() before send() or receive()" << endl;
    }
public:
    void send()
    {
        call_once(of, &CommSlot::open, this);
        cout << "send()" << endl;
    }
    void receive()
    {
        call_once(of, &CommSlot::open, this);
        cout << "receive()" << endl;
    }
};

CommSlot & getInstance()
{
    static CommSlot Cs;
    return Cs;
}

void f1()
{
    for(int i = 0; i < 10; i++)
    {
        getInstance().send();
    }
}

void f2()
{
    for(int i = 0; i < 10; i++)
    {
        getInstance().receive();
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

