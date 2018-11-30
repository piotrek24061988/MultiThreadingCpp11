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
class lock_free_queue {
private:
    struct node {
        shared_ptr<T> data;
        node * next;

        node() : next(nullptr){}
    };

    atomic<node*> head;
    atomic<node*> tail;

    node * pop_head() {
        node * const old_head = head.load();
        if(old_head==tail.load()) {//Empty queue
            return nullptr;
        }
        head.store(old_head->next);
        return old_head;
    }
public:
    lock_free_queue() : head(new node), tail(head.load()){
        cout << "This lock_free_queue is really lock free: " << boolalpha << atomic_is_lock_free(&head) << endl;
    }

    lock_free_queue(const lock_free_queue & other) = delete;
    lock_free_queue & operator=(const lock_free_queue & other) = delete;

    ~lock_free_queue() {
        while(node * const old_head = head.load()) {
            head.store(old_head->next);
            delete old_head;
        }
    }

    shared_ptr<T> pop() {
        node * old_head = pop_head();
        if(!old_head) {
            return shared_ptr<T>();
        }
        shared_ptr<T> const res(old_head->data);
        delete old_head;
        return res;
    }

    void push(T new_value) {
        shared_ptr<T> new_data(make_shared<T>(new_value));
        node * p = new node;//Alocate new node as fake node.
        node * const old_tail = tail.load();
        old_tail->data.swap(new_data);
        old_tail->next=p;
        tail.store(p);
    }
};

void f1(lock_free_queue<int> & st) {
    for(auto & i : {1, 2, 3, 4, 5}) {
        st.push(i);
    }
}

void f2(lock_free_queue<int> & st) {
    for(auto & i : {6, 7, 8, 9, 10}) {
        st.push(i);
    }
}

void f3(lock_free_queue<int> & st) {
    for(int i = 0; i < 5;) {
        if(auto a = st.pop()) {
            cout << i++ << ":" << *a <<  ",  ";
        }
    }
}

void f4(lock_free_queue<int> & st) {
    for(int i = 0; i < 5;) {
        if(auto a = st.pop()) {
            cout << i++ << ":" << *a <<  ",  ";
        }
    }
}

int main()
{
    lock_free_queue<int> st;

    thread t1(f1, ref(st)); thread t2(f2, ref(st));

    thread t3(f3, ref(st)); thread t4(f4, ref(st));

    t1.join(); t2.join();

    t3.join(); t4.join();

    cout << endl;
}




