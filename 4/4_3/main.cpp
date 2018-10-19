#include <iostream>
#include <future>
#include <mutex>
#include <chrono>
#include <thread>
using namespace std;

int returnVal()
{
    static int i;
    this_thread::sleep_for(3s);
    cout << "i: " << i << endl;
    return i++;
}

void doSomeWork()
{
    for(int i = 0; i < 4; i++)
    {
        cout << "doSomeWork" << i << endl;
        this_thread::sleep_for(1s);
    }
}

int main()
{
    cout << "main()" << endl;
    future<int> the_answer = async(returnVal);
    doSomeWork();
    cout << "returned vaule is: " << the_answer.get() << endl;

    return 0;
}

