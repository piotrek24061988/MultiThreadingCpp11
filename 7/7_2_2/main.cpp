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
        atomic<unsigned> threads_in_pop;

        struct node {
            shared_ptr<T> data;
            node * next = nullptr;

            node(const T & data_) : data(make_shared<T>(data_)){}
        };

        atomic <node*> to_be_deleted;

        atomic<node*> head {nullptr};

        static void delete_nodes(node * nodes) {
            while(nodes) {
                node * next = nodes->next;
                delete nodes;
                nodes = next;
            }
        }

        void try_reclaim(node *old_head) {
            //Current node is used only by current pop (1 thread) so
            //can be deleted togeter with nodes waiting to be deleted.
            if(threads_in_pop == 1) {
                //Get list of nodes waiting to be deleted.
                node * nodes_to_delete = to_be_deleted.exchange(nullptr);
                //If there was no new pop instance in meantime called
                if(!--threads_in_pop) {
                    //Delete all nodes waiting for being deleted.
                    delete_nodes(nodes_to_delete);
                //If there was a new pop called in meantime deleting is
                //not safe so we need to push them back to be deleted later.
                } else if(nodes_to_delete) {
                    chain_pending_nodes(nodes_to_delete);
                }
                delete old_head;//delete current node directly.
            } else {//Current node is used by pop in other thread so can
                    //be only added to list of nodes to be deleted late.
                chain_pending_node(old_head);
                --threads_in_pop;
            }
        }

        void chain_pending_nodes(node * nodes) {
            node * last = nodes;
            //find end of list with node to be deleted
            while(node * const next = last->next) {
                last = next;
            }
            //Add to be deleted list to end of nodes list
            //and save first element of this chain as a
            //new node_to_be_deleted.
            chain_pending_nodes(nodes, last);
        }

        void chain_pending_nodes(node *first, node * last) {
            last->next = to_be_deleted;
            while(!to_be_deleted.compare_exchange_weak(last->next, first));
        }

        void chain_pending_node(node * n) {
            chain_pending_nodes(n, n);
        }

public:
        void push(const T & data) {
            node * new_node = new node(data); //Create new node
            new_node->next = head.load(); //Set new node next to current head.
            //If head is equal to new_node->next (what was requested in previous line
            //but could be modified by other thread), set head to new_node.
            while(!head.compare_exchange_weak(new_node->next, new_node));
        }

        //Version 3
        //issues: there is kind of internal garbage_collecter for elements
        //pop from stack because they can't be deleted directly. But if
        //there is have load of stack by using pop function all the time
        //garbage will never be removed and will grow till the end of
        //available memory.
        shared_ptr<T> pop() {
            ++threads_in_pop;
            node * old_head = head.load(); //Read value from current head.
            //If head is equeal to old_head(what was requested in precious line
            //but could be modified by other thread), set head to head->next,
            //to next node on the list.
            while(old_head && !head.compare_exchange_weak(old_head, old_head->next));

            shared_ptr<T> res;
            if(old_head) {
                res.swap(old_head->data);
            }
            try_reclaim(old_head);
            return res;
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
}

