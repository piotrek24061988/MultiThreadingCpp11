#include <iostream>
#include <thread>
using namespace std;

void hello()
{
    for(int i = 0; i < 10; i++)
    {
        cout << "Witaj w swiecie wspolbieznosci: " << i << endl;
    }
}

//2 dzialajace watki + wyrazenie lambda + joinable
int main()
{
    cout << "Przed uruchomieniem watkow" << endl;
    std::thread t1{
                []()->void
                {
                    for(int i = 0; i < 10; i++)
                    {
                        cout << "Witaj w swiecie wspolbieznosci wyrazenie lambda: " << i << endl;
                    }
                }};
    cout << "Pomiedzy uruchomieniem watkow" << endl;
    std::thread t2{hello};
    if(t1.joinable())
    {
            cout << "t1 joinable" << endl;
            t1.join();
    }
    if(t1.joinable())
    {
            cout << "t1 joinable" << endl;
            t1.join();
    }
    if(t2.joinable())
    {
            cout << "t2 joinable" << endl;
            t2.join();
    }
    if(t2.joinable())
    {
            cout << "t2 joinable" << endl;
            t2.join();
    }
    cout << "Po zakonczeniu watkow" << endl;
    return 0;
}
