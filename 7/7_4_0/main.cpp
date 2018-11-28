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

    struct counted_node_ptr {
        int8_t external_count;//Increased with every read of node.
        node * ptr;
    };

    struct node {
        shared_ptr<T> data;
        atomic<int8_t> internal_count;//Decreased when read of node is finished.
        counted_node_ptr next;

        node(T const & data_) : data(make_shared<T>(data_)), internal_count(0){}
    };

    atomic<counted_node_ptr> head;

    void increase_head_count(counted_node_ptr & old_counter) {
        counted_node_ptr new_counter;
        do {
            new_counter = old_counter;
            ++new_counter.external_count;
        } while(!head.compare_exchange_strong(old_counter, new_counter));
        old_counter.external_count = new_counter.external_count;
    }

public:
    lock_free_stack() {
        cout << "this lock free stack is really lock free: " << boolalpha << atomic_is_lock_free(&head) << endl;
    }

    ~lock_free_stack() {
        while(pop());
    }

    void push(T const & data) {
        counted_node_ptr new_node;     //Create new node
        new_node.ptr = new node(data); //with data and internal_count 0
        new_node.external_count = 1;   //and external_count 1
        new_node.ptr->next = head.load(); //Asign current head as a new_node->ptr.
        //Make new node as a new head. External_count 1 because head is only refference to current new node.
        while(!head.compare_exchange_weak(new_node.ptr->next, new_node));
    }

    //external_count increased, internal_count decreased
    shared_ptr<T> pop() {
        counted_node_ptr old_head = head.load();//1)Read current head
        while(1) {
            increase_head_count(old_head);//2)Increase external_count to current head atomically.
            node * const ptr = old_head.ptr;
            if(!ptr) {//If this node is empty this is the end of list and there is no elements to process.
                return shared_ptr<T>();
            }
            //If node is not empty we can try to delete this node from stack.
            if(head.compare_exchange_strong(old_head, ptr->next)) {
                //Current thread take over this node.
                shared_ptr<T> res;
                res.swap(ptr->data);

                int const count_increase = old_head.external_count-2;
                //There is no reference to node and it can be deleted.
                if(ptr->internal_count.fetch_add(count_increase) == -count_increase) {
                    delete ptr;
                }
                //At the end return poped data.
                return res;
            //If current thread was not able to take over node it means that other thread
            //delete this node or add new node to stack.
            //We need to decrease cout of references to node that we tried to delete.
            } else if(ptr->internal_count.fetch_add(-1)==1) {
                //If current node was the last who have reference to this node we
                //can delete it.
                ptr->internal_count.load();
                delete ptr;
            }

        }
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

