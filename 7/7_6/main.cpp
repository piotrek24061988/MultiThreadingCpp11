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
    struct node;

    struct counted_node_ptr {
        int external_count;
        node * ptr;
    };

    atomic<counted_node_ptr> head;
    atomic<counted_node_ptr> tail;

    struct node_counter {
        int internal_count    : 30;
        int external_counters : 2; //Up to 2 external counters;
    };

    struct node {
        atomic<T*> data;
        atomic<node_counter> count;
        counted_node_ptr next;

        node(){
            data.store(nullptr);
            node_counter new_count;
            //Each new node after succesfull adding to queue is pointed by
            //2 references, 1 in node tail and 1 in ptr next in previous node.
            new_count.internal_count = 0;
            new_count.external_counters = 2;
            count.store(new_count);

            next.ptr = nullptr;
            next.external_count = 0;
        }

        void release_ref() {
            node_counter old_counter = count.load(memory_order_relaxed);
            node_counter new_counter;
            do {
                new_counter = old_counter;
                --new_counter.internal_count;
            } while(!count.compare_exchange_strong(old_counter, new_counter, memory_order_acquire, memory_order_relaxed));

            //If our thread and any other thread not use this node anymore we can delete it.
            if(!new_counter.internal_count && !new_counter.external_counters) {
                delete this;
            }
        }
    };

    static void increase_external_count(atomic<counted_node_ptr> & counter, counted_node_ptr & old_counter) {
        counted_node_ptr new_counter;

        do {
            new_counter = old_counter;
            ++new_counter.external_count;
        } while(!counter.compare_exchange_strong(old_counter, new_counter, memory_order_acquire, memory_order_relaxed));

        old_counter.external_count = new_counter.external_count;
    }

    static void free_external_counter(counted_node_ptr & old_node_ptr) {
        node * const ptr = old_node_ptr.ptr;
        int const count_increase = old_node_ptr.external_count - 2;

        node_counter old_counter = ptr->count.load(memory_order_relaxed);

        node_counter new_counter;
        do {
            new_counter = old_counter;
            --new_counter.external_counters;
            new_counter.internal_count += count_increase;
        } while(!ptr->count.compare_exchange_strong(old_counter, new_counter, memory_order_acquire, memory_order_relaxed));

        //If our thread and any other thread not use this node anymore we can delete it.
        if(!new_counter.internal_count && !new_counter.external_counters) {
            delete ptr;
        }
    }

public:
    lock_free_queue() {
        cout << "This lock_free_queue is really lock free1: " << boolalpha << atomic_is_lock_free(&head) << endl;
        counted_node_ptr counted_node;
        counted_node.ptr = new node;
        counted_node.external_count = 1;

        head.store(counted_node);
        tail.store(head);
    }
    ~lock_free_queue() {
        counted_node_ptr old_head = head.load();
        while(node * const old_node = old_head.ptr) {
            head.store(old_node->next);
            delete old_node;
            old_head = head.load();
        }
    }

    lock_free_queue(const lock_free_queue & other) = delete;
    lock_free_queue & operator=(const lock_free_queue & other) = delete;

    void push(T new_value) {
        unique_ptr<T> new_data(make_unique<T>(new_value));
        counted_node_ptr new_next;
        new_next.ptr = new node;
        new_next.external_count = 1;
        counted_node_ptr old_tail = tail.load();//We get old tail.

        while(1) {
            increase_external_count(tail, old_tail);//We increase its counter because we are using it.
            T * old_data = nullptr;
            //We try to get data from old tail.
            if(old_tail.ptr->data.compare_exchange_strong(old_data, new_data.get())) {
                old_tail.ptr->next = new_next;//We update tail
                old_tail = tail.exchange(new_next);
                free_external_counter(old_tail);//At the end we decrement counter.
                new_data.release();
                break;
            }//If it doesn't work we decrement counter.
            old_tail.ptr->release_ref();
        }
    }

    unique_ptr<T> pop() {
        counted_node_ptr old_head = head.load(memory_order_relaxed);//We get old head.
        while(1) {
            increase_external_count(head, old_head);//We increase its counter.
            node * const ptr = old_head.ptr;
            if(ptr == tail.load().ptr) {
                ptr->release_ref();//If there is nothing to pop we decrease counter
                return unique_ptr<T>();//and return empty result
            }
            //If we pop 1 element and right now update head.
            if(head.compare_exchange_strong(old_head, ptr->next)) {
                T * const res = ptr->data.exchange(nullptr);
                free_external_counter(old_head);//We decrease counter at the end.
                return unique_ptr<T>(res);//Returning head value
            }
            ptr->release_ref();//If we wasnt able to get head we decrease counter
                               //and repeat everything once again.
        }
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
