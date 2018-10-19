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
    lock_guard<mutex> guard(some_mutex);

    some_list.push_back(new_value);
    cout << "Added to list: " << new_value << endl;
}

bool list_contains(int value_to_find)
{
    lock_guard<mutex> guard(some_mutex);

    return find(some_list.begin(), some_list.end(), value_to_find) != some_list.end();
}

void f1()
{
    vector<int> vec {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    for(auto & x : vec)
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
    thread t1(f2);
    thread t2(f1);
    t1.detach();
    t2.join();
    return 0;
}

