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
        struct node
        {
            shared_ptr<T> data;
            node * next = nullptr;

            node(const T & data_)
                : data(make_shared<T>(data_))
            {}
        };

        atomic<node*> head;
public:
        void push(const T & data)//thread safe
        {
            node * new_node = new node(data);//utworzenie nowego wezla
            new_node->next = head.load();//ustawienie wskaznika next tak aby wskazywal dotychczasowy head
            while(!head.compare_exchange_weak(new_node->next, new_node));//ustawienie head tak aby wskazywal nowy wezel
        }
        shared_ptr<T> pop()
        {
            node * old_head = head.load();
            while(old_head && !head.compare_exchange_weak(old_head, old_head->next));
            return old_head ? old_head->data : shared_ptr<T>();
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

