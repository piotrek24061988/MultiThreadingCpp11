#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
using namespace std;

void f1(atomic<bool> & b)
{
    bool expected = false;
    cout << "f1 begining, expected: " << expected << endl;

   if(b.compare_exchange_weak(expected, true))
   {
       cout << "f1 set b, expected: " << expected << endl;
   }
   else
   {
       cout << "f1 not set b, expected: " << expected << endl;
   }

   if(b.compare_exchange_weak(expected, true))
   {
       cout << "f1 set b, expected: " << expected << endl;
   }
   else
   {
       cout << "f1 not set b, expected: " << expected << endl;
   }

    cout << "f1 end, expected: " << expected << endl;
}

int main()
{
    atomic<bool> b;
    b.store(false);

    thread t1(f1, ref(b));

    t1.join();

    return 0;
}

