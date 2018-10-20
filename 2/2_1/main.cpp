#include <iostream>
#include <thread>
using namespace std;

class thread_guard
{
    thread & t;

public:
    explicit thread_guard(thread & t_)
    : t(t_)
    {}

    ~thread_guard()
    {
            if(t.joinable())
            {
                t.join();
            }
    }

    thread_guard(const thread_guard &) = delete;
    thread_guard & operator=(const thread_guard &) = delete;
};

void f()
{
        int some_local_state = 0;

        thread t{[&some_local_state]()->void
                 {
                     for(int i = 0; i < 100; i++)
                     {
                         cout << i << " ";
                         if(!(i % 10))
                         {
                             cout << endl;
                         }
                     }
                 } 
                };
        thread_guard g(t);

        for(int i = 0; i < 100; i++)
        {
            cout << "f()";
            if(!(i % 10))
            {
                cout << endl;
            }
        }
        cout << endl;
}

int main()
{
    f();
    return 0;
}

