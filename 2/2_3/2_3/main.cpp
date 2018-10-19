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
    f(thread([](){cout << "Hello World" << endl;}));
    thread t([](){
                     for(int i = 0; i < 10; i++)
                     {
                        cout << "Hello" << endl;
                      }
                  }
             );
    f(std::move(t));
    return 0;
}

