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
            shared_ptr<node> next;

            node(const T & data_) : data(make_shared<T>(data_)) {}
        };

        shared_ptr<node> head;
public:
        lock_free_stack() {
            cout << "is shared_ptr atomic without locks? " << boolalpha << atomic_is_lock_free(&head) << endl;
            if(atomic_is_lock_free(&head)) {
                cout << "So this lock_free_stack is realy lock free" << endl;
                cout << "because shared_ptr doesnt use internal locks to be atomic." << endl;
            } else {
                cout << "So this lock_free_stack is really not lock free" << endl;
                cout << "because shared_ptr use internal locks to be atomic." << endl;
            }
        }

        void push(const T & data) {
            shared_ptr<node> new_node = make_shared<node>(data);
            new_node->next = head;
            while(!atomic_compare_exchange_weak(&head, &new_node->next, new_node));
        }

        shared_ptr<T> pop() {
            shared_ptr<node> old_head = atomic_load(&head);
            while(old_head && !atomic_compare_exchange_weak(&head, &old_head, old_head->next));
            return old_head ? old_head->data : shared_ptr<T>();
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
        if(auto a = st.pop()) {
            cout << *a <<  " ";
        }
    }
}

void f4(lock_free_stack<int> & st) {
    for(int i = 0; i < 5; i++) {
        if(auto a = st.pop()) {
            cout << *a <<  " ";
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
}


