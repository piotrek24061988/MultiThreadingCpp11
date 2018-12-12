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
    atomic<unsigned> count;//Total number of threads
    atomic<unsigned> spaces;//Free places for threads
    atomic<unsigned> generation;

public:
    explicit barrier(unsigned count_) : count(count_), spaces(count_), generation(0){}

    void wait() {
        unsigned const my_generation = generation;
        if(!--spaces) {//Adding new waiting thread decrease number of free places.
            spaces = count.load();//If no more pleaces reasign initial value and
            ++generation;//let know waiting threads that they can continue they work.
        } else {
            while(generation==my_generation) {//If barier achieved, stop waiting.
                this_thread::yield();//Do not waiste processor time when waiting.
            }
        }
    }

    void done_waiting() {//Decrease total number of waiting threads to use
        --count;         //new lower value when current barier achieved.
        if(!--spaces) {//Decrease number of free places.
            spaces=count.load();//If no more pleaces reasign initial value and
            ++generation;//let know waiting threads that they can continue they work.
        }
    }
};

void f(barrier & b, string s)
{
    cout << s << " before" << endl;
    this_thread::sleep_for(1s);
    b.wait();
    cout << s <<" after" << endl;
}

int main()
{
    barrier b(3);

    thread t1(f, ref(b), "t1"); thread t2(f, ref(b), "t2"); thread t3(f, ref(b), "t3");

    t1.join(); t2.join(); t3.join();

    cout << endl << endl;

    thread t4(f, ref(b), "t4"); thread t5(f, ref(b), "t5");
    this_thread::sleep_for(1s);
    b.done_waiting();

    t4.join(); t5.join();
}

