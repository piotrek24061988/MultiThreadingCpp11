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
            T data;
            node * next = nullptr;

            node(const T & data_)
                : data(data_)
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
        /*
        T pop()//non thread safe
        {
            node * old_head = head.load();
            node * old_next = head.load()->next;
            head.store(old_next);
            head.load()->next = old_next->next;
            T tmp = old_head->data;
            delete old_head;
            return tmp;
        }
        */
        T pop()
        {
            node * old_head = head.load();
            T tmp = old_head->data;
            while(!head.compare_exchange_weak(old_head, old_head->next));
            delete old_head;
            return tmp;
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
        cout << st.pop() <<  " ";
    }
}

void f4(lock_free_stack<int> & st)
{
    for(int i = 0; i < 5; i++)
    {
        cout << st.pop() <<  " ";
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

