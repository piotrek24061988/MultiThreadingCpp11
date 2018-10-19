#include <iostream>
#include <vector>
#include <thread>
using namespace std;

int main()
{
    vector<int> i {100, 200, 300};

    for(auto & x : i)
    {
        cout << "initial: " << x << endl;
    }

    thread t1([&i](){
                    for(auto & x : i)
                    {
                        cout << "inside thread: " << x << endl;
                        x++;
                    }
    });
    t1.join();

    for(auto & x : i)
    {
        cout << "at the end: " << x << endl;
    }

    return 0;
}

