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

template<typename T>
class queue
{
private:
    struct node
    {
        shared_ptr<T> data;
        unique_ptr<node> next;
    };

    unique_ptr<node> head;
    node * tail;

public:

    queue() : head(new node), tail(head.get()){}
    queue(const queue & other) = delete;
    queue & operator=(const queue & other) = delete;

    shared_ptr<T> try_pop()
    {
        if(head.get() == tail)
        {
            return shared_ptr<T>();
        }
        shared_ptr<T> const res(head->data);// store tmp head node data;
        unique_ptr<node> const old_head = move(head);//store head node in tmp object
        head = move(old_head->next);//move head to next element
        return res;// return tmp head node data
    }

    void push(T new_value)
    {
        shared_ptr<T> new_data(make_shared<T>(move(new_value))); //store new data
        unique_ptr<node> p(new node);
        tail->data = new_data; // assign new data to current tail empty node
        node * const new_tail = p.get(); // new tail will be new empty node
        tail->next = move(p); //so tail is new empty node
        tail = new_tail;      //so tail is new empty node
    }
};

int main()
{
    queue<int> q;
    q.push(1);
    cout << *(q.try_pop()) << endl << endl;

    q.push(2);
    cout << *(q.try_pop()) << endl << endl;

    q.push(3);
    q.push(4);
    cout << *(q.try_pop()) << endl;
    cout << *(q.try_pop()) << endl << endl;

    q.push(5);
    q.push(6);
    q.push(7);

    if(auto a = q.try_pop())
    {
        cout << *a << endl;
    }
    else
    {
        cout << "no val" << endl;
    }

    if(auto a = q.try_pop())
    {
        cout << *a << endl;
    }
    else
    {
        cout << "no val" << endl;
    }

    if(auto a = q.try_pop())
    {
        cout << *a << endl;
    }
    else
    {
        cout << "no val" << endl;
    }

    if(auto a = q.try_pop())
    {
        cout << *a << endl;
    }
    else
    {
        cout << "no val" << endl;
    }

   return 0;
}
