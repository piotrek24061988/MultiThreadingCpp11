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

//Program konczy sie czasem przed zakonczeniem pracy watku
int main()
{
    cout << "Przed uruchomieniem watku" << endl;
    std::thread t(hello);//Nie czekamy na koniec watku wiec moze sie zdazyc
                         //ze program skonczy prace przed zakonczeniem pracy watku
    t.detach();
    cout << "Po uruchomieniu watku" << endl;
    for(int i = 0; i < 10000; i++);
    cout << "Za jakis czas" << endl;
    return 0;
}
