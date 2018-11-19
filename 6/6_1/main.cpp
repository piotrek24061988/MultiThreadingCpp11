#include <iostream>
#include <mutex>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <exception>
#include <stack>
using namespace std;

struct empty_stack : exception {};

template <typename T>
class threadsafe_stack {
private:
    stack<T> data;
    mutable mutex m;
public:
    threadsafe_stack(){}
    threadsafe_stack(const threadsafe_stack & other) {
        lock_guard<mutex> lock(other.m);
        data = other.data;
    }
    threadsafe_stack & operator=(const threadsafe_stack & other) = delete;

    void push(T new_value) {
        lock_guard<mutex> lock(m);
        data.push(move(new_value));
    }

    shared_ptr<T> pop() {
        lock_guard<mutex> lock(m);
        if(data.empty()) {
            throw empty_stack();
        }
        shared_ptr<T> const res(make_shared<T>(move(data.top())));
        data.pop();
        return res;
    }

    void pop(T & value) {
        lock_guard<mutex> lock(m);
        if(data.empty()) {
            throw empty_stack();
        }
        value = move(data.top());
        data.pop();
    }

    bool empty() const {
        lock_guard<mutex> lock(m);
        return data.empty();
    }
};

threadsafe_stack<int> tSs;
atomic<bool> endf;

void f1() {
    endf = false;
    for(auto & a : {1, 2, 3, 4, 5, 6, 7, 8 , 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19}) {
        tSs.push(a);
         this_thread::sleep_for(100ms);
    }
    this_thread::sleep_for(1s);
    endf = true;
}

void f2() {
    while(!endf) {
        if(!tSs.empty()) {
            try {
               cout << "f2: " << *(tSs.pop()) << endl;
            }
            catch(...) {
               cout << "f2: pop thrown exception" << endl;
            }
        }
    }
}

void f3() {
    while(!endf) {
        if(!tSs.empty()) {
            try {
                int val;
                tSs.pop(val);
                cout << "f3: " << val << endl;
            }
            catch(...) {
               cout << "f3: pop thrown exception" << endl;
            }
        }
    }
}

void f4() {
    while(!endf) {
        if(!tSs.empty()) {
            try {
                int val;
                tSs.pop(val);
                cout << "f4: " << val << endl;
            }
            catch(...) {
               cout << "f4: pop thrown exception" << endl;
            }
        }
    }
}

int main() {
   thread t2(f2);
   thread t3(f3);
   thread t4(f4);

   thread t1(f1);

   t1.join(); t2.join(); t3.join(); t4.join();
}

