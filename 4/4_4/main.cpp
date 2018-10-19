#include <iostream>
#include <future>
#include <mutex>
#include <chrono>
#include <thread>
using namespace std;

struct X
{
    void foo(int i, const string & s)
    {
        this_thread::sleep_for(1s);
        cout << "foo, i: " << i << ", s: " << s << endl;
    }

    string bar(const string & s)
    {
        this_thread::sleep_for(1s);
        cout << "bar s: " << s << endl;
        return s + "_bar";
    }
};

struct Y
{
    double operator()(double d)
    {
        this_thread::sleep_for(1s);
        cout << "operator(), d: " << d << endl;
        return d + 2;
    }
};

X baz(X & x)
{
    cout << "baz" << endl;
    this_thread::sleep_for(1s);
    return x;
}

class move_only
{
public:
    move_only()
    {
        cout << "move_only" << endl;
    }

    move_only(move_only&& o)
    {
        cout << "move_only&&" << endl;
    }
    move_only(const move_only & o) = delete;
    move_only & operator=(move_only && o)
    {
        cout << "move_only=" << endl;
        return o;
    }
    move_only & operator=(const move_only & o) = delete;

    void operator()()
    {
        this_thread::sleep_for(1s);
        cout << "operator()" << endl;
    }
};

int main()
{
    X x;
    auto f1 = async(launch::async, &X::foo, &x, 42, "witaj");//launch::async zadanie wykonane w tle w osobny
                                                             //watku
    auto f2 = async(&X::bar, &x, "zegnaj");//decyzja kompilatora czy zadanie wykonywane w tle w osobny watku

    Y y;
    auto f3 = async(launch::deferred, Y(), 3.14);//launch::deferred zadanie wykonane w biezacym watku
                                                 //w momencie wywolania get();
    auto f4 = async(launch::async | launch::deferred, ref(y), 2.718);//decyzja kompilatora czy zadanie
                                                                     //wykonywane w tle w osobny watku

    async(baz, ref(x));

    auto f5 = async(move_only());

    f1.get();
    cout << f2.get() << endl;
    cout << f3.get() << endl;
    cout << f4.get() << endl;
    f5.get();

    return 0;
}

