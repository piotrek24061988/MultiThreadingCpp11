#include <iostream>
#include <thread>
#include <memory>
using namespace std;

void f(int & i)
{
        for(int j = 0; j < 5; j++)
        {
           cout << ++i << endl;
        }
}

class F2
{
public:
    void f2(int & j)
    {
        for(int k = 0; k < 5; k++)
        {
            cout << ++j << endl;
        }
    }
};

void f3(unique_ptr<int> a)
{
        cout << "a: " << *a << endl;
}

void f4(int && i)
{
        for(int j = 0; j < 5; j++)
        {
           cout << "f4: " << ++i << endl;
        }
}

int main()
{
    int k = 1;
    cout << "k: " << k << endl;
    thread t(f, ref(k));
    t.join();
    cout << "k: " << k << endl;

    F2 ka;
    thread l(&F2::f2, &ka, ref(k));
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
    else
    {
        cout << "z thread was sucesfully moved to x thread" << endl;
    }

    int v4 = 1;
    thread t4(f4, move(v4));
    t4.join();
    cout << "after f4: " << v4 << endl;

    return 0;
}

