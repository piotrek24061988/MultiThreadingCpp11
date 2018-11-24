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
        struct node {
            shared_ptr<T> data;
            node * next = nullptr;

            node(const T & data_) : data(make_shared<T>(data_)){}
        };

        atomic<node*> head {nullptr};
public:
        void push(const T & data) {
            node * new_node = new node(data); //Create new node
            new_node->next = head.load(); //Set new node next to current head.
            //If head is equal to new_node->next (what was requested in previous line
            //but could be modified by other thread), set head to new_node.
            while(!head.compare_exchange_weak(new_node->next, new_node));
        }
        //Version 2
        //issues: memleak
        shared_ptr<T> pop() {
            node * old_head = head.load(); //Read value from current head.
            //If head is equeal to old_head(what was requested in precious line
            //but could be modified by other thread), set head to head->next,
            //to next node on the list.
            while(old_head && !head.compare_exchange_weak(old_head, old_head->next));
            return old_head ? old_head->data : shared_ptr<T>();
            //memleak nobody is releasing old_head;
        }
};

void f1(lock_free_stack<int> & st) {
    for(auto & i : {1, 2, 3, 4, 5}) {
        st.push(i);
    }
}

void f2(lock_free_stack<int> & st) {
    for(auto & i : {6, 7, 8, 9, 10}) {
        st.push(i);
    }
}

void f3(lock_free_stack<int> & st) {
    for(int i = 0; i < 5; i++) {
        if(auto i = st.pop()) {
         cout << "f3: " << *i <<  " ";
        }
    }
}

void f4(lock_free_stack<int> & st) {
    for(int i = 0; i < 5; i++) {
        if(auto i = st.pop()) {
           cout << "f4: " << *i <<  " ";
        }
    }
}

int main()
{
    lock_free_stack<int> st;

    thread t1(f1, ref(st)); thread t2(f2, ref(st));

    thread t3(f3, ref(st)); thread t4(f4, ref(st));

    t1.join(); t2.join();

    t3.join(); t4.join();
    cout << endl;

    return 0;
}

