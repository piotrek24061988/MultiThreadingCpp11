#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>
using namespace std;

int main()
{
    vector<thread> threads;
    for(int i = 0; i < thread::hardware_concurrency(); i++)
    {
        threads.push_back(thread([](int k){cout << "k: " << k << endl;}, i));
    }
    for_each(threads.begin(), threads.end(), mem_fn(&thread::join));

    return 0;
}

