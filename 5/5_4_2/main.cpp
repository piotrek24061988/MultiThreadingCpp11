#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
using namespace std;


int main()
{
    int a[5] = {1, 2, 3, 4, 5};
    atomic<int*> p(a);
    cout << *p.fetch_add(2) << endl;
    cout << *p << endl;
    cout << *p.fetch_sub(1) << endl;
    cout << *p << endl;

}
