#include <iostream>
#include <mutex>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <exception>
#include <stack>
#include <condition_variable>
using namespace std;

template<typename T>
class queue
{
private:
    struct node {
        shared_ptr<T> data;
        unique_ptr<node> next;
    };

    mutex head_mutex;
    unique_ptr<node> head;

    mutex tail_mutex;
    node * tail;

    node * get_tail() {
        lock_guard<mutex> tail_lock(tail_mutex);
        return tail;
    }

    unique_ptr<node> pop_head() {
        lock_guard<mutex> head_lock(head_mutex);

        if(head.get() == get_tail()) {
            return nullptr;
        }

        unique_ptr<node> old_head = move(head);
        head = move(old_head->next);
        return old_head;
    }

public:

    queue() : head(new node), tail(head.get()){}
    queue(const queue & other) = delete;
    queue & operator=(const queue & other) = delete;

    shared_ptr<T> try_pop() {
        unique_ptr<node> old_head = pop_head();
        return old_head ? old_head->data : shared_ptr<T>();
    }

    void push(T new_value) {
        shared_ptr<T> new_data = make_shared<T>(move(new_value));
        unique_ptr<node> p(new node);
        node * const new_tail = p.get();
        lock_guard<mutex> tail_lock(tail_mutex);
        tail->data = new_data;
        tail->next = move(p);
        tail = new_tail;
    }
};

void f1(queue<int> & q, atomic<bool> & a) {
    for(auto & i : {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19}) {
        q.push(i);
        this_thread::sleep_for(10ms);
    }
    this_thread::sleep_for(1s);
    a = true;
}

void f2(queue<int> & q, atomic<bool> & b) {
    for(auto & i : {-1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15, -16, -17, -18, -19}) {
        q.push(i);
        this_thread::sleep_for(10ms);
    }
    this_thread::sleep_for(1s);
    b = true;
}

void f3(queue<int> & q, atomic<bool> & a, atomic<bool> & b) {
    while(!a || !b) {
        if(auto a = q.try_pop()) {
            cout << "f3: " << *a << endl;
        }
        this_thread::sleep_for(10ms);
    }
}

void f4(queue<int> & q, atomic<bool> & a, atomic<bool> & b) {
    while(!a || !b) {
        if(auto a = q.try_pop()) {
            cout << "f4: " << *a << endl;
        }
        this_thread::sleep_for(10ms);
    }
}

int main() {
    queue<int> q;
    if(auto a = q.try_pop()) {
         cout << *a << endl;
    } else {
         cout << "no val" << endl;
    }

    q.push(1);
    cout << *(q.try_pop()) << endl << endl;

    q.push(2);
    cout << *(q.try_pop()) << endl << endl;

    q.push(3);
    q.push(4);
    cout << *(q.try_pop()) << endl;
    cout << *(q.try_pop()) << endl << endl;

    q.push(5);
    q.push(6);
    q.push(7);
    if(auto a = q.try_pop()) {
        cout << *a << endl;
    } else {
        cout << "no val" << endl;
    }
    if(auto a = q.try_pop()) {
        cout << *a << endl;
    } else {
        cout << "no val" << endl;
    }
    if(auto a = q.try_pop()) {
        cout << *a << endl;
    } else {
        cout << "no val" << endl;
    }
    if(auto a = q.try_pop()) {
        cout << *a << endl;
    } else {
        cout << "no val" << endl;
    }

    atomic<bool> a{false};
    atomic<bool> b{false};

    thread t1(f1, ref(q), ref(a)); thread t2(f2, ref(q), ref(b));

    thread t3(f3, ref(q), ref(a), ref(b)); thread t4(f4, ref(q), ref(a), ref(b));

    t1.join(); t2.join(); t3.join(); t4.join();
}
