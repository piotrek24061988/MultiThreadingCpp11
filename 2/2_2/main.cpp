#include <iostream>
#include <thread>
#include <memory>
using namespace std;

void f(int & i)
{
        for(int j = 0; j < 100; j++)
        {
           i++;
        }
}

class K
{
public:
    void f2(int & j)
    {
        for(int k = 0; k < 5; k++)
        {
            cout << "fa" << endl;
            j++;
        }
    }
};

void f3(unique_ptr<int> a)
{
        cout << "a: " << *a << endl;
}

int main()
{
    int k = 1;
    cout << "k: " << k << endl;
    thread t(f, ref(k));
    t.join();
    cout << "k: " << k << endl;

    K ka;
    thread l(&K::f2, &ka, ref(k));
    l.join();
    cout << "k: " << k << endl;

    unique_ptr<int> b {new int{3}};
    thread z(f3, move(b));
    thread x = std::move(z);
    x.join();
    if(b != nullptr)
    {
        cout << "after move(b): " << *b << endl;
    }


    return 0;
}

