#include <iostream>
#include <memory>
#include <mutex>
#include <stack>
#include <chrono>
#include <thread>
using namespace std;

struct empty_stack : exception
{
};

template <typename T>
class threadsafe_stack
{
private:
    stack<T> data;
    mutable mutex m;
public:
    threadsafe_stack(){}
    threadsafe_stack(const threadsafe_stack & other)
    {
        lock_guard<mutex> lock(other.m);
        data = other.data;
    }

    threadsafe_stack & operator=(const threadsafe_stack & other ) = delete;

    void push(T new_value)
    {
        lock_guard<mutex> lock(m);
        data.push(new_value);
    }

    shared_ptr<T> pop()
    {
        lock_guard<mutex> lock(m);
        if(data.empty())
        {
            throw empty_stack();
        }
        shared_ptr<T> const res{make_shared<T>(data.top())};
        data.pop();
        return res;
    }
    void pop(T & value)
    {
        lock_guard<mutex> lock(m);
        if(data.empty())
        {
            throw empty_stack();
        }
        value = data.top();
        data.pop();
    }
    bool empty() const
    {
        lock_guard<mutex> lock(m);
        return data.empty();
    }
};

void f1(threadsafe_stack<int> & myStack)
{
    int i = 30;
    while(i--)
    {
        try
        {
            cout << "pop() : " << *(myStack.pop()) << endl;
        }
        catch(...)
        {
            cout << "empty stack exception" << endl;
        }
        this_thread::sleep_for(1s);
    }
}

void f2(threadsafe_stack<int> & myStack)
{
    int i = 10;
    while(i--)
    {
        myStack.push(i);
        this_thread::sleep_for(3s);
    }
}

int main()
{
    threadsafe_stack<int> myStack;

    thread t1(f1, ref(myStack));
    thread t2(f2, ref(myStack));

    t1.join();
    t2.join();

    return 0;
}

