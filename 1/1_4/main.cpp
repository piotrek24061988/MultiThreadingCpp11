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

//2 dzialajace watki + wyrazenie lambda
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
    t1.join();
    t2.join();
    cout << "Po zakonczeniu watkow" << endl;
    return 0;
}
