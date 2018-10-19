#include <iostream>
#include <mutex>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <exception>
#include <stack>
#include <condition_variable>
using namespace std;

template <typename T>
class threadsafe_stack
{
private:
    stack<T> data;
    mutable mutex m;
    condition_variable data_cond;
public:
    threadsafe_stack(){}
    threadsafe_stack(const threadsafe_stack & other) = delete;
    threadsafe_stack & operator=(const threadsafe_stack & other) = delete;

    void push(T new_value)
    {
        {
            lock_guard<mutex> lock(m);
            data.push(move(new_value));
        }
        data_cond.notify_one();
    }

    shared_ptr<T> pop()
    {
        shared_ptr<T> res;
        {
            unique_lock<mutex> lock(m);
            data_cond.wait(lock, [this]{return !data.empty();});
            res = make_shared<T>(move(data.top()));
            data.pop();
        }
        return res;
    }

    void pop(T & value)
    {
        unique_lock<mutex> lock(m);
        data_cond.wait(lock, [this]{return !data.empty();});
        value = move(data.top());
        data.pop();
    }
};

void f0(threadsafe_stack<int> & tSs, atomic<bool> & endf)
{
    endf = false;
    for(auto & a : {0, 1, 2, 3, 4, 5, 6, 7, 8, 9})
    {
        tSs.push(a);
    }
    this_thread::sleep_for(1s);
    endf = true;
}

void f1(threadsafe_stack<int> & tSs, atomic<bool> & endf)
{
    endf = false;
    for(auto & a : {10, 11, 12, 13, 14, 15, 16, 17, 18, 19})
    {
        tSs.push(a);
    }
    this_thread::sleep_for(1s);
    endf = true;
}

void f2(threadsafe_stack<int> & tSs, const atomic<bool> & endf1, const atomic<bool> & endf2)
{
    while(!endf1 || !endf2)
    {
        cout << "f2: " << *(tSs.pop()) << endl;
    }
}

void f3(threadsafe_stack<int> & tSs, const atomic<bool> & endf1, const atomic<bool> & endf2)
{
    while(!endf1 || !endf2)
    {
        int val;
        tSs.pop(val);
        cout << "f3: " << val << endl;
    }
}

int main()
{
   threadsafe_stack<int> tSs;
   atomic<bool> endf1 {false};
   atomic<bool> endf2 {false};

   thread t3(f3, ref(tSs), ref(endf1), ref(endf2));
   thread t2(f2, ref(tSs), ref(endf1), ref(endf2));
   thread t1(f1, ref(tSs), ref(endf1));
   thread t0(f0, ref(tSs), ref(endf2));
   t3.detach();
   t2.detach();
   t1.join();
   t0.join();
   return 0;
}
