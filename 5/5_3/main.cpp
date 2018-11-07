#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
using namespace std;

int main()
{
    atomic<bool> b;
    b.store(false);        //Set atomic flag to false so we expect
    bool expected = false; //there is false.

   cout << "begining, expected: " << expected << ", value: " << b.load() << endl;
   if(b.compare_exchange_weak(expected, true))//Change value to true.
   {   //Seting was succesfull so b = true and expected = false.
       cout << "1 set sucessfull, expected: " << expected << ", value: " << b.load() << endl;
   }
   else
   {   //Set was unsuccesfull, may happen for compare_exchange_weak
       //so b = false, expected = false
       cout << "1 set unsucessfull, expected: " << expected << ", value: " << b.load() << endl;
   }

   if(b.compare_exchange_strong(expected, true))
   {
       //If previous set was succesfull this should not happed.
       //Otherwise set was done sucesfully this time
       cout << "2 set sucessfull, expected: " << expected << ", value: " << b.load() << endl;
   }
   else
   {
       //If previous set was succesfull this time there was not set
       //so expected value was updated from false to true.
       //If previous set was unsuccesfull this won be called
       //because strong never fail.
       cout << "2 set unsucessfull, expected: " << expected << ", value: " << b.load() << endl;
   }
}

