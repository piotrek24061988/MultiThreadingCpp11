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
class queue {
private:
    struct node {
        shared_ptr<T> data;
        unique_ptr<node> next;
    };

    mutex head_mutex;
    unique_ptr<node> head;

    mutex tail_mutex;
    node * tail;

    condition_variable data_cond;

    node * get_tail() {
        lock_guard<mutex> tail_lock(tail_mutex);
        return tail;
    }

    unique_ptr<node> pop_head() {
        unique_ptr<node> old_head = move(head);
        head = move(old_head->next);
        return old_head;
    }

    unique_lock<mutex> wait_for_data() {
        unique_lock<mutex> head_lock(head_mutex);
        data_cond.wait(head_lock, [&](){return head.get()!=get_tail();});
        return move(head_lock);
    }

    unique_ptr<node> wait_pop_head() {
        unique_lock<mutex> head_lock(wait_for_data());
        return pop_head();
    }

    unique_ptr<node> wait_pop_head(T & value) {
        unique_lock<mutex> head_lock(wait_for_data());
        value = move(*head->data);
        return pop_head();
    }

    unique_ptr<node> try_pop_head() {
        lock_guard<mutex> head_lock(head_mutex);
        if(head.get() == get_tail()) {
            return unique_ptr<node>();
        }
        return pop_head();
    }

    unique_ptr<node> try_pop_head(T & value) {
        lock_guard<mutex> head_lock(head_mutex);
        if(head.get() == get_tail()) {
            return unique_ptr<node>();
        }
        value = move(*head->data);
        return pop_head();
    }

public:
    queue() : head(new node), tail(head.get()){}
    queue(const queue & other) = delete;
    queue & operator=(const queue & other) = delete;

    shared_ptr<T> try_pop() {
        unique_ptr<node> old_head = try_pop_head();
        return old_head ? old_head->data : shared_ptr<T>();
    }

    bool try_pop(T & value) {
        unique_ptr<node> old_head = try_pop_head(value);
        return old_head;
    }

    shared_ptr<T> wait_and_pop() {
        unique_ptr<node> const old_head = wait_pop_head();
        return old_head->data;
    }

    void wait_and_pop(T & value) {
        unique_ptr<node> const old_head = wait_pop_head(value);
    }

    void push(T new_value) {
        shared_ptr<T> new_data = make_shared<T>(move(new_value));
        unique_ptr<node> p(new node);
        {
            lock_guard<mutex> tail_lock(tail_mutex);
            tail->data = new_data;
            node * const new_tail = p.get();
            tail->next = move(p);
            tail = new_tail;
        }
        data_cond.notify_one();
    }

    void empty() {
        lock_guard<mutex> head_lock(head_mutex);
        return (head.get() == get_tail());
    }
};

atomic<bool> a{false};
atomic<bool> b{false};
atomic<bool> c{false};

void f0(queue<int> & q) {
    for(auto & i : {-1, -2, -3, -4, -5, -6, -7, -8, -9, -10}) {
        q.push(i);
        this_thread::sleep_for(500ms);
    }
    this_thread::sleep_for(500ms);
    a = true;
}

void f1(queue<int> & q) {
    for(auto & i : {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}) {
        q.push(i);
        this_thread::sleep_for(500ms);
    }
    this_thread::sleep_for(500ms);
    b = true;
}

void f2(queue<int> & q) {
    for(auto & i : {11, 12, 13, 14, 15, 16, 17, 18, 19}) {
        q.push(i);
        this_thread::sleep_for(500ms);
    }
    this_thread::sleep_for(500ms);
    c = true;
}

void f3(queue<int> & q) {
    while(!a || !b || !c) {
        if(auto a = q.try_pop()) {
            cout << "f3: " << *a << endl;
        }
    }
}

void f4(queue<int> & q) {
    while(!a || !b || !c) {
        if(auto a = q.try_pop()) {
            cout << "f4: " << *a << endl;
        }
    }
}

void f5(queue<int> & q) {
    while(!a || !b || !c) {
            cout << "f5: " << *q.wait_and_pop() << endl;
    }
}

void f6(queue<int> & q) {
    while(!a || !b || !c) {
            cout << "f6: " << *q.wait_and_pop() << endl;
    }
}

int main()
{
    //thread safe queue used in one thread
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

    cout << endl << endl << endl;

    //thread safe queue used in many threads
    thread t0(f0, ref(q));
    thread t1(f1, ref(q));
    thread t2(f2, ref(q));

    thread t3(f3, ref(q));
    thread t4(f4, ref(q));

    thread t5(f5, ref(q));
    thread t6(f6, ref(q));

    t0.join(); t1.join(); t2.join();

    t3.join(); t4.join();

    t5.detach(); t6.detach();
}
