#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
using namespace std;


void f2(unique_lock<mutex> l1, int & val)
{
    for(int i = 0; i < 5; i++)
    {
        cout << val++ << endl;
    }
}

void f1(mutex & m1, int & val)
{
    unique_lock<mutex> l1(m1);
    for(int i = 0; i < 5; i++)
    {
        cout << val++ << endl;
    }
    f2(move(l1), val);
}

int main()
{
    int i = 3;
    mutex m;

    thread t1(f1, ref(m), ref(i));
    thread t2(f1, ref(m), ref(i));
    thread t3(f1, ref(m), ref(i));
    t1.join();
    t2.join();
    t3.join();


    return 0;
}

