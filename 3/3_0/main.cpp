#include <iostream>
#include <thread>
#include <mutex>
#include <list>
#include <algorithm>
#include <chrono>
#include <vector>
using namespace std;

list<int> some_list;
mutex some_mutex;

void add_to_list(int new_value)
{
    some_mutex.lock();
    some_list.push_back(new_value);
    some_mutex.unlock();
    cout << "Added to list: " << new_value << endl;
}

bool list_contains(int value_to_find)
{
    some_mutex.lock();
    bool f = find(some_list.begin(), some_list.end(), value_to_find) != some_list.end();
    some_mutex.unlock();
    return f;
}

void f1()
{
    for(auto & x : {1, 2, 3, 4, 5, 6, 7, 8, 9, 10})
    {
        add_to_list(x);
        this_thread::sleep_for(1s);
    }
}

void f2()
{
    while(!list_contains(10))
    {
        this_thread::sleep_for(1s);
    }
    cout << "Founded!" << endl;
}

int main()
{
    thread t1(f1);
    thread t2(f2);
    t1.join();
    t2.join();
    return 0;
}
