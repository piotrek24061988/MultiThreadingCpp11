#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>
using namespace std;

void do_work(int k)
{
    static int i = 0;
    i++;
    cout << "i: " << i << ", k: " << k << endl;
}

int main()
{
    vector<thread> threads;
    for(int i = 0; i < thread::hardware_concurrency(); i++)
    {
        threads.push_back(thread(do_work, i));
    }
    for_each(threads.begin(), threads.end(), mem_fn(&thread::join));

    return 0;
}

