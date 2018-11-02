#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <utility>
#include <future>
#include <exception>
using namespace std;

class myException : public exception
{
public:
    const char * what()  const throw()
    {
        return "my exception";
    }
};

template<typename T>
void f1(future<T> & fut)
{
    for(int i = 0; i < 10; i++)
    {
        promise<T> pr;
        fut = pr.get_future();
        if(i != 9)
        {
           pr.set_value(i);
        }
        else
        {
            try
            {
               throw myException();
            }
            catch(...)
            {
               pr.set_exception(std::current_exception());
            }
        }
        this_thread::sleep_for(1s);
    }
}

template<typename T>
void f2(future<T> & fut)
{
    for(int i = 0; i < 10;)
    {
        if(fut.valid())
        {
           fut.wait();
           try
           {
              cout << fut.get() << endl;
           }
           catch(myException & e)
           {
               cout << e.what() << endl;
           }
           i++;
        }
    }
}

int main()
{
    future<int> fut;
    thread t1(f1<int>, ref(fut));
    thread t2(f2<int>, ref(fut));
    t1.join();
    t2.join();

    return 0;
}

