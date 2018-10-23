#include <iostream>
#include <memory>
#include <mutex>
#include <stack>
#include <chrono>
#include <thread>
#include <list>
using namespace std;

template <typename T>
void swap(list<T> & a, list<T> & b)
{
    auto c = a;
    a = b;
    b = c;
}

template <typename T>
ostream & operator<<(ostream & os, const list<T> & z)
{
    for(auto & x : z)
    {
        cout << x << " ";
    }
    cout << endl;
}

template <typename T>
class X
{
private:
    list<T> & obj;
    mutex m;
public:
    X(list<T> & obj_)
        : obj(obj_)
    {}

    friend void swap(X & lhs, X & rhs)
    {
        if(&lhs == &rhs)
        {
            return;
        }
        lock(lhs.m, rhs.m);
        lock_guard<mutex> locka(lhs.m, adopt_lock);
        lock_guard<mutex> lockb(rhs.m, adopt_lock);
        swap(lhs.obj, rhs.obj);
    }
};

void f1(X<int> & x1, X<int> & x2)
{
    swap(x1, x2);
}

int main()
{
    list<int> a{0, 1, 2, 3, 4};
    list<int> b{5, 6, 7, 8, 9};

    cout << a << endl;
    cout << b << endl;

    X<int> x1(a);
    X<int> x2(b);

    thread t1(f1, ref(x1), ref(x2));
    thread t2(f1, ref(x2), ref(x1));
    thread t3(f1, ref(x1), ref(x2));
    thread t4(f1, ref(x2), ref(x1));
    thread t5(f1, ref(x1), ref(x2));
    thread t6(f1, ref(x2), ref(x1));
    thread t7(f1, ref(x1), ref(x2));
    thread t8(f1, ref(x2), ref(x1));
    thread t9(f1, ref(x1), ref(x2));
    t1.join();t2.join();t3.join();t4.join();t5.join();t6.join();t7.join();t8.join();t9.join();

    cout << a << endl;
    cout << b << endl;


    return 0;
}

