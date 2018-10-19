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

//Funkcja main czeka na poprawne zakonczenie watku
int main()
{
    cout << "Przed uruchomieniem watkow" << endl;
    std::thread t(hello);
    t.join();//Czekamy na poprawne zakonczenie watku
    cout << "Po zakonczeniu watkow" << endl;
    return 0;
}
