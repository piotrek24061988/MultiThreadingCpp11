#include <iostream>
#include <future>
#include <mutex>
#include <chrono>
#include <thread>
#include <future>
using namespace std;

int returnVal()
{
    int i;
    for(i = 0; i < 4; i++)
    {
        cout << "returnVal i: " << i << endl;
        this_thread::sleep_for(2s);
    }
    return i;
}

void doSomeWork()
{
    for(int i = 0; i < 4; i++)
    {
        cout << "doSomeWork i: " << i << endl;
        this_thread::sleep_for(1s);
    }
}

int main()
{
    future<int> the_answer = async(returnVal);
    doSomeWork();
    cout << "returned vaule is: " << the_answer.get() << endl;
}

