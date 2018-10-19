#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
using namespace std;

class Y
{
private:
    int some_detail;
    mutable mutex m;
public:
    Y(int sd)
        : some_detail(sd)
    {}

    friend bool operator==(Y const  & lhs, Y const & rhs)
    {
        if(&lhs == &rhs)
        {
            return true;
        }
        int const lhs_value = lhs.get_detail();
        int const rhs_value = rhs.get_detail();
        return lhs_value == rhs_value;
    }

    int get_detail() const
    {
        lock_guard<mutex> lock_a(m);
        return some_detail;
    }
    void set_detail(int val)
    {
        lock_guard<mutex> lock_a(m);
        some_detail = val;
    }
};

Y y1{1}, y2{1};

void f1()
{
    for(int i = y1.get_detail(); i < 100; i +=2)
    {
        if(y1 == y2)
        {
            cout << "f1 values equal: "  << y1.get_detail() << ", " << y2.get_detail() << endl;
        }
        else
        {
            cout << "f1 values not equeal: "  << y1.get_detail() << ", " << y2.get_detail() << endl;
        }
        this_thread::sleep_for(2s);
        y1.set_detail(i);
    }
}

void f2()
{
    for(int i = y2.get_detail(); i < 100; i++)
    {
        if(y1 == y2)
        {
            cout << "f2 values equal: " << y1.get_detail() << ", " << y2.get_detail() << endl;
        }
        else
        {
            cout << "f2 values not equeal: " <<  y1.get_detail() << ", " << y2.get_detail() << endl;
        }
        this_thread::sleep_for(1s);
        y2.set_detail(i);
    }
}

int main()
{
    thread t1(f1);
    thread t2(f2);
    t1.join();
    t2.join();
    return 0;
}

