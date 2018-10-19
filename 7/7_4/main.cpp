#include <iostream>
#include <mutex>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <exception>
#include <stack>
#include <condition_variable>
#include <list>
#include <boost/thread/shared_mutex.hpp>
#include <algorithm>
using namespace std;

template<typename T>
class lock_free_stack
{
private:
    struct node;

    struct counted_node_ptr
    {
        int external_count;
        node * ptr;
    };

    struct node
    {
        shared_ptr<T> data;
        atomic<int> internal_count;
        counted_node_ptr next;

        node(T const & data_)
            :
            data(make_shared<T>(data_)),
            internal_count(0)
        {
        }
    };

    atomic<counted_node_ptr> head;

    void increase_head_count(counted_node_ptr & old_counter)
    {
        counted_node_ptr new_counter;

        do
        {
            new_counter = old_counter;
            ++new_counter.external_count;
        }
        while(!head.compare_exchange_strong(old_counter, new_counter, memory_order_acquire, memory_order_relaxed));

        old_counter.external_count = new_counter.external_count;
    }

public:
    ~lock_free_stack()
    {
        while(pop());
    }

    void push(T const & data)
    {
        counted_node_ptr new_node;
        new_node.ptr = new node(data);
        new_node.external_count = 1;
        new_node.ptr->next = head.load(memory_order_relaxed);
        while(!head.compare_exchange_weak(new_node.ptr->next, new_node, memory_order_release, memory_order_relaxed));
    }

    shared_ptr<T> pop()
    {
        counted_node_ptr old_head = head.load(memory_order_relaxed);
        while(1)
        {
            increase_head_count(old_head);
            node * const ptr = old_head.ptr;
            if(!ptr)
            {
                return shared_ptr<T>();
            }
            if(head.compare_exchange_strong(old_head, ptr->next, memory_order_relaxed))
            {
                shared_ptr<T> res;
                res.swap(ptr->data);

                int const count_increase = old_head.external_count-2;

                if(ptr->internal_count.fetch_add(count_increase, memory_order_release) == -count_increase)
                {
                    delete ptr;
                }
                return res;
            }
            else if(ptr->internal_count.fetch_add(-1, memory_order_relaxed)==1)
            {
                ptr->internal_count.load(memory_order_acquire);
                delete ptr;
            }

        }
    }
};

void f1(lock_free_stack<int> & st)
{
    for(auto & i : {1, 2, 3, 4, 5})
    {
        st.push(i);
    }
}

void f2(lock_free_stack<int> & st)
{
    for(auto & i : {6, 7, 8, 9, 10})
    {
        st.push(i);
    }
}

void f3(lock_free_stack<int> & st)
{
    for(int i = 0; i < 5; i++)
    {
        cout << *st.pop() <<  " ";
    }
}

void f4(lock_free_stack<int> & st)
{
    for(int i = 0; i < 5; i++)
    {
        cout << *st.pop() <<  " ";
    }
}

int main()
{
    lock_free_stack<int> st;

    thread t1(f1, ref(st));
    thread t2(f2, ref(st));
    t1.join();
    t2.join();

    thread t3(f3, ref(st));
    thread t4(f4, ref(st));
    t3.join();
    t4.join();
    cout << endl;

    return 0;
}


