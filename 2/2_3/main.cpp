#include <iostream>
#include <thread>
using namespace std;

void f(std::thread t)
{
    if(t.joinable())
    {
        t.join();
    }
}

int main()
{
    f(thread([](){cout << "Hello World 1" << endl;}));

    thread t([](){
                     for(int i = 0; i < 3; i++)
                     {
                        cout << "Hello World 2" << endl;
                      }
                  }
             );
    f(std::move(t));
    return 0;
}

