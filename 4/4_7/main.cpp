#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <utility>
#include <future>
#include <exception>
using namespace std;

template<typename T>
void f1(future<T> & fut)
{
    promise<T> pr;
    fut = pr.get_future();
    this_thread::sleep_for(5s);
    pr.set_value(5);
}

template<typename T>
void f2(shared_future<T> fut)
{
    auto start = chrono::high_resolution_clock::now();
    if(fut.valid())
    {
        cout << "f2: " << fut.get() << endl;
    }
    auto stop = chrono::high_resolution_clock::now();
    cout << "f2 takes: " << chrono::duration_cast<chrono::milliseconds>(stop - start).count() << " ms" << endl;
}

template<typename T>
void f3(shared_future<T> fut)
{
    if(fut.valid())
    {
        cout << "f3: " << fut.get() << endl;
    }
}

template<typename T>
void f4(shared_future<T> fut)
{
    if(fut.valid())
    {
        if(fut.wait_for(std::chrono::seconds(3)) == future_status::ready)
        {
            cout << "f4: " << fut.get() << endl;
        }
        else
        {
             cout << "f4: timeout" << endl;
        }
    }
}

template<typename T>
void f5(shared_future<T> fut)
{
    if(fut.valid())
    {
        if(fut.wait_for(std::chrono::seconds(6)) == future_status::ready)
        {
            cout << "f5: " << fut.get() << endl;
        }
        else
        {
             cout << "f5: timeout" << endl;
        }
    }
}

template<typename T>
void f6(shared_future<T> fut)
{
    if(fut.valid())
    {
        if(fut.wait_until(chrono::high_resolution_clock::now() + chrono::seconds(4)) == future_status::ready)
        {
            cout << "f6: " << fut.get() << endl;
        }
        else
        {
             cout << "f6: timeout" << endl;
        }
    }
}

int main()
{
    future<int> fut;
    thread t1(f1<int>, ref(fut));
    while(!fut.valid());
    shared_future<int> fut1(move(fut));
    shared_future<int> fut2(fut1);
    shared_future<int> fut3(fut1);
    shared_future<int> fut4(fut1);
    shared_future<int> fut5(fut1);
    thread t2(f2<int>, fut1);
    thread t3(f3<int>, fut2);
    thread t4(f4<int>, fut3);
    thread t5(f5<int>, fut4);
    thread t6(f6<int>, fut5);
    if(!fut.valid())
    {
        cout << "fut is not valid anymore" << endl;
    }
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();

    return 0;
}

