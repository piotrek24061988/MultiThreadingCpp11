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
using namespace std;

class barrier
{
    atomic<unsigned> count;
    atomic<unsigned> spaces;
    atomic<unsigned> generation;
public:
    explicit barrier(unsigned count_) : count(count_), spaces(count_), generation(0)
    {}

    void wait()
    {
        unsigned const gen = generation.load();
        if(!--spaces)
        {
            spaces = count.load();
            ++generation;
        }
        else
        {
            while(gen==generation.load())
            {
                this_thread::yield();
            }
        }
    }

    void done_waiting()
    {
        --count;
        if(!--spaces)
        {
            spaces=count.load();
            ++generation;
        }
    }
};

void f1(barrier & b)
{
    this_thread::sleep_for(1s);
    cout << "f1 before" << endl;
    b.wait();
    cout << "f1 after" << endl;
}

void f2(barrier & b)
{
    this_thread::sleep_for(2s);
    cout << "f2 before" << endl;
    b.wait();
    cout << "f2 after" << endl;
}

void f3(barrier & b)
{
    this_thread::sleep_for(3s);
    cout << "f3 before" << endl;
    b.wait();
    cout << "f3 after" << endl;
}

void f4(barrier & b)
{
    this_thread::sleep_for(4s);
    cout << "f4 before" << endl;
    b.wait();
    cout << "f4 after" << endl;
}

void f5(barrier & b)
{
    this_thread::sleep_for(5s);
    cout << "f5 before" << endl;
    b.wait();
    cout << "f5 after" << endl;
}

void f6(barrier & b)
{
    this_thread::sleep_for(5s);
    cout << "f6 before" << endl;
    b.done_waiting();
    cout << "f6 after" << endl;
}


int main()
{
    barrier b(5);
    thread t1(f1, ref(b));
    thread t2(f2, ref(b));
    thread t3(f3, ref(b));
    thread t4(f4, ref(b));
    thread t5(f5, ref(b));
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();

    cout << endl << endl;

    barrier b2(7);
    thread tr1(f1, ref(b2));
    thread tr2(f2, ref(b2));
    thread tr3(f3, ref(b2));
    thread tr4(f4, ref(b2));
    thread tr5(f5, ref(b2));
    thread tr6(f6, ref(b2));
    thread tr7(f6, ref(b2));
    tr1.join();
    tr2.join();
    tr3.join();
    tr4.join();
    tr5.join();
    tr6.join();
    tr7.join();

    return 0;
}

