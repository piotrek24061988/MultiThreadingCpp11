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

constexpr unsigned max_hazard_pointers = 100;

struct hazard_pointer {
    atomic<thread::id> id;//Id of thread using node pointed
    atomic<void*> pointer;//by pointer and cant be deleted.
};
hazard_pointer hazard_pointers[max_hazard_pointers];

class hp_owner
{
    hazard_pointer * hp;

public:
    hp_owner(const hp_owner &) = delete;
    hp_owner & operator=(const hp_owner &) = delete;

    hp_owner(): hp{nullptr} {
        //Find unused hazzardPtr from hazzardPtrList and asing it to current thread.
        for(unsigned i = 0; i < max_hazard_pointers; ++i) {
            thread::id old_id;
            if(hazard_pointers[i].id.compare_exchange_strong(old_id, this_thread::get_id())) {
                hp = &hazard_pointers[i];
                break;
            }
        }
        if(!hp) {
            throw runtime_error("No available hazard pointer in list");
        }
    }

    atomic<void*>& get_pointer() {
        return hp->pointer;
    }

    ~hp_owner() {
        hp->pointer.store(nullptr);
        //Default id indicating unsuded hazardPtr.
        hp->id.store(thread::id());
    }
};

//Each thread is owner of separated hazzardPtr.
atomic<void*>& get_hazard_pointer_for_current_thread() {
    thread_local static hp_owner hazard;
    return hazard.get_pointer();
}

//Check if any thread has hazzardPtr pointing to node
//provided as argument.
bool outstanding_hazard_pointers_for(void * p) {
    for(unsigned i = 0; i < max_hazard_pointers; ++i) {
        if(hazard_pointers[i].pointer.load() == p) {
            return true;
        }
    }
    return false;
}

template<typename T>
void do_delete(void * p) {
    delete static_cast<T*>(p);
}

struct data_to_reclaim {
    void * data;
    function<void(void*)> deleter;
    data_to_reclaim * next;

    template<typename T>
    data_to_reclaim(T * p): data(p), deleter(&do_delete<T>), next(0){}

    ~data_to_reclaim() {
        deleter(data);
    }
};

atomic<data_to_reclaim*> nodes_to_reclaim;

void add_to_reclaim_list(data_to_reclaim* node) {
    node->next = nodes_to_reclaim.load();
    while(!nodes_to_reclaim.compare_exchange_weak(node->next, node));
}

//Add node to list of nodes to be deleted later
template<typename T>
void reclaim_later(T *data) {
    add_to_reclaim_list(new data_to_reclaim(data));
}

//Delete all nodes added to list of nodes to be deleted later
//if they don't have owners (it there is no hazzardPointer for them)/
void delete_nodes_with_no_hazards() {
    data_to_reclaim * current = nodes_to_reclaim.exchange(nullptr);
    while(current) {
        data_to_reclaim * const next = current->next;
        if(!outstanding_hazard_pointers_for(current->data)) {
            delete current;
        } else {
            add_to_reclaim_list(current);
        }
        current = next;
    }
}

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
        lock_free_stack() {
             cout << "is shared_ptr atomic without locks? " << boolalpha << atomic_is_lock_free(&head) << endl;
        }

        void push(const T & data) {
            node * new_node = new node(data); //Create new node
            new_node->next = head.load(); //Set new node next to current head.
            //If head is equal to new_node->next (what was requested in previous line
            //but could be modified by other thread), set head to new_node.
            while(!head.compare_exchange_weak(new_node->next, new_node));
        }
        //Version 4
        //Poping is quite slow because of complexity of used algorithm
        //to clean garbage.
        shared_ptr<T> pop() {
            atomic<void*> & hp = get_hazard_pointer_for_current_thread();
            node * old_head = head.load();
            do {
                node * temp;
                do {//HazardPtr should point to head.
                    temp = old_head;
                    //Old head cant be deleted by other thread because was
                    //added to HazardPtr.
                    hp.store(old_head);
                    old_head = head.load();
                //To double check if we really stored head in HazzardPtr and
                //head was not modified in meantime by other thread.
                } while(old_head != temp);
            } while(old_head && !head.compare_exchange_strong(old_head, old_head->next));
            //Clearing hazzard pointer which is no longer needed because we already get node.
            hp.store(nullptr);
            shared_ptr<T> res;
            if(old_head) {
                res.swap(old_head->data);
                //Before current node will be deleted we need to check if there are
                //other threads hazzard pointers pointing to this node.
                if(outstanding_hazard_pointers_for(old_head)) {
                    reclaim_later(old_head);//If yes mark to be deleted later.
                } else {
                    delete old_head;//If no delete directly.
                }
                //Checks all nodes marked for later delete with reclaim_later
                //function if there are ready to be deleted right now.
                delete_nodes_with_no_hazards();
            }
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

    return 0;
}
