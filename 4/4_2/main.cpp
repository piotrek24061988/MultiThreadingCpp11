#include <iostream>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <queue>
#include <thread>
using namespace std;

template<typename T>
class threadsafe_queue
{
private:
    mutex mut;
    queue<T> data_queue;
    condition_variable data_cond;

public:
    threadsafe_queue()
    {}

    threadsafe_queue(threadsafe_queue const & other)
    {
        lock_guard<mutex> lk(other.mut);
        data_queue = other.data_queue;
    }

    void push(T new_value)
    {
        std::lock_guard<mutex> lk(mut);
        data_queue.push(new_value);
        data_cond.notify_all();
    }

    void wait_and_pop(T& value)
    {
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [this](){return !data_queue.empty();});
        value = data_queue.front();
        data_queue.pop();
    }

    shared_ptr<T> wait_and_pop()
    {
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [this](){return !data_queue.empty();});
        shared_ptr<T> res = make_shared<T>(data_queue.front());
        data_queue.pop();
        return res;
    }

    bool try_pop(T& value)
    {
        lock_guard<mutex> lk(mut);
        if(data_queue.empty())
        {
            return false;
        }
        value = data_queue.front();
        data_queue.pop();
        return true;
    }

    shared_ptr<T> try_pop()
    {
        lock_guard<mutex> lk(mut);
        if(data_queue.empty())
        {
            return shared_ptr<T>(nullptr);
        }
        shared_ptr<T> res = make_shared<T>(data_queue.front());
        data_queue.pop();
        return res;
    }

    bool empty() const
    {
        std::lock_guard<mutex> lk(mut);
        return data_queue.empty();
    }
};

threadsafe_queue<int> data_queue;

void f1()
{
    for(int i = 0; i < 5; i++)
    {
        data_queue.push(i);
        this_thread::sleep_for(3s);
    }
}

void f2()
{
    int j;
    for(int i = 0; i < 5; i++)
    {
        data_queue.wait_and_pop(j);
        data_queue.push(j);
        cout << "f2: " << j << endl;
        this_thread::sleep_for(1s);
    }
}


void f3()
{
    while(true)
    {
       int k;
       if(data_queue.try_pop(k))
       {
          cout << "f3: " << k << endl;
          if(k == 4)
          {
              break;
          }
       }
    }
}

int main()
{
    thread t1(f1);
    thread t2(f2);
    thread t3(f3);
    t1.join();
    t2.join();
    t3.join();
    return 0;
}

