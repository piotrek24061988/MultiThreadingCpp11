#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
using namespace std;

class Foo
{
public:
    int a;
};


int main()
{
    Foo some_array[5] = {1, 2, 3, 4, 5};
    atomic<Foo*> p(some_array);
    Foo* x = p.fetch_add(2);
    cout << x->a << endl;
    cout << p.load()->a << endl;

    Foo * y = p-=1;
    cout << y->a << endl;
    cout << p.load()->a << endl;

    Foo* z = p.fetch_sub(1);
    cout << z->a << endl;
    cout << p.load()->a << endl;

    return 0;
}

