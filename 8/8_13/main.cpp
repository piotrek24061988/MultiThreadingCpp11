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
        unsigned const my_generation = generation;
        if(!--spaces)
        {
            spaces = count.load();
            ++generation;
        }
        else
        {
            while(generation==my_generation)
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

    return 0;
}

