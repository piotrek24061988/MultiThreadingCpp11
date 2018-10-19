#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
using namespace std;

void f(bool flag, int j)
{
    static thread_local unsigned int i;//Change comments to se the differnece
    //static unsigned int i;

    while(i < j)
    {
        i++;
        cout << i << endl;

        if(flag)
        {
            this_thread::sleep_for(1s);
        }
        else
        {
            this_thread::sleep_for(2s);
        }
    }
}

void f2(bool state, vector<int> k)
{
    for(auto & x : k)
    {
        f(state, x);
    }
}

int main()
{
    vector<int> a(10, 20);
    thread t1(f2, true, a);
    thread t2(f2, false, a);
    t1.join();
    t2.join();
    return 0;
}

