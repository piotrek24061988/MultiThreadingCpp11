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

/*
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
        shared_ptr<T> const res(head->data);// pop node
        unique_ptr<node> const old_head = move(head);//store old head
        head = move(old_head->next);//head = next to old head
        return res;// return node
    }

    void push(T new_value)
    {
        shared_ptr<T> new_data(make_shared<T>(move(new_value)));
        unique_ptr<node> p(new node);
        tail->data = new_data;
        node * const new_tail = p.get();
        tail->next = move(p);
        tail = new_tail;
    }
};
*/
template<typename T>
class queue
{
private:
    struct node
    {
        T data;
        unique_ptr<node> next;

        node(T data_) : data(move(data_)){}
    };

    unique_ptr<node> head;
    node * tail = nullptr;

public:
    queue(){}
    queue(const queue & other) = delete;
    queue & operator=(const queue & other) = delete;

    void push(T new_value)
    {
        unique_ptr<node> p(make_unique<node>(new_value));
        node * new_tail = p.get();
        if(tail)
        {
            tail->next = move(p);
        }
        else
        {
            head = move(p);
        }
        tail = new_tail;
    }

    shared_ptr<T> try_pop()
    {
        if(!head)
        {
            return shared_ptr<T>();
        }
        shared_ptr<T> const res(make_shared<T>(move(head->data)));
        unique_ptr<node> const old_head = move(head);
        head = move(old_head->next);
        if(tail->next == head)
        {
            tail = nullptr;
        }
        return res;
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
