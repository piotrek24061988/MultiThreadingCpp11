#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <utility>
#include <future>
#include <exception>
using namespace std;

class myException : public exception {
public:
    const char * what()  const throw() {
        return "my exception";
    }
};

void f1(promise<int> & pr, bool exceptionCase)
{
    if(!exceptionCase) {
        this_thread::sleep_for(1s); //Do some work.
        pr.set_value(3);//provide result to waiting thread.
    } else {
        try {
           this_thread::sleep_for(1s);//Do some work.
           throw myException();//Work failed.
        } catch(...) { //Notify working thread about this fail.
           pr.set_exception(std::current_exception());
        }
    }
}

void f2(shared_future<int> & fut)
{
    fut.wait();
    try {
        cout << fut.get() << endl;
    } catch(myException & e) {
        cout << e.what() << endl;
    }
}

void f3(shared_future<int> & fut)
{
    fut.wait();
    try {
        cout << fut.get() << endl;
    } catch(myException & e) {
        cout << e.what() << endl;
    }
}

int main()
{
    promise<int> pr1;
    future<int> fut1 = pr1.get_future();
    shared_future<int> sfut1(move(fut1));
    thread t1(f1, ref(pr1), false);
    thread t2(f2, ref(sfut1));
    thread t3(f3, ref(sfut1));
    t1.join();
    t2.join();
    t3.join();

    promise<int> pr2;
    future<int> fut2 = pr2.get_future();
    shared_future<int> sfut2(move(fut2));
    thread t4(f1, ref(pr2), true);
    thread t5(f2, ref(sfut2));
    thread t6(f3, ref(sfut2));
    t4.join();
    t5.join();
    t6.join();

    return 0;
}

