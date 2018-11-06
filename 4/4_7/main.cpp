#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <utility>
#include <future>
#include <exception>
using namespace std;

template<typename T>
void f1(promise<T> & pr)
{
    this_thread::sleep_for(5s);
    pr.set_value(5);
}

template<typename T>
void f2(shared_future<T> & fut)
{
    auto start = chrono::high_resolution_clock::now();
    cout << "f2: " << fut.get() << endl;
    auto stop = chrono::high_resolution_clock::now();
    cout << "f2 takes: " << chrono::duration_cast<chrono::milliseconds>(stop - start).count() << " ms" << endl;
}

template<typename T>
void f3(shared_future<T> & fut)
{
    //Wait for feature 3s and check if we have valid result or timeout.
    if(fut.wait_for(std::chrono::seconds(3)) == future_status::ready) {
        cout << "f3: " << fut.get() << endl;
    } else {
        cout << "f3: timeout" << endl;
    }
}

template<typename T>
void f4(shared_future<T> & fut)
{
    //Wait for feature 3s and check if we have valid result or timeout.
    if(fut.wait_for(std::chrono::seconds(6)) != future_status::timeout) {
        cout << "f4: " << fut.get() << endl;
    } else {
        cout << "f4: timeout" << endl;
    }
}

template<typename T>
void f5(shared_future<T> & fut)
{
    //Wait for feature till (curent time + 5s) and check if we have valid result or timeout.
    if(fut.wait_until(chrono::high_resolution_clock::now() + chrono::seconds(5)) == future_status::ready) {
        cout << "f5: " << fut.get() << endl;
    } else {
        cout << "f5: timeout" << endl;
    }
}

int main()
{
    promise<int> pr;
    shared_future<int> fut1(pr.get_future());
    shared_future<int> fut2(fut1);
    shared_future<int> fut3(fut1);
    shared_future<int> fut4(fut1);

    if(!fut1.valid()) return -1; //Is feature already paired with promise.

    thread t1(f1<int>, ref(pr));
    thread t2(f2<int>, ref(fut1));
    thread t3(f3<int>, ref(fut2));
    thread t4(f4<int>, ref(fut3));
    thread t5(f5<int>, ref(fut4));

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();

    return 0;
}

