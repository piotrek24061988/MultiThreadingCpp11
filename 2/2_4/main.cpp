#include <iostream>
#include <thread>
using namespace std;

class scoped_thread
{
    thread t;
public:
    explicit scoped_thread(thread t_)
             : t(move(t_))
    {
        cout << "1" << endl;
        if(!t.joinable())
        {
            throw logic_error("brak watku");
        }
    }
    ~scoped_thread()
    {
        cout << "-1" << endl;
        t.join();
    }
    scoped_thread(const scoped_thread &) = delete;
    scoped_thread & operator=(const scoped_thread &) = delete;
};

struct func
{
    int & i;

    func(int & i_) : i(i_){cout << "1" << endl;}

    void operator()()
    {
        cout << "2" << endl;
        for(int j = 0; j < i; j++)
        {
               cout << ".";
        }
    }
};

void f()
{
    cout << "0" << endl;
    int some_local_state = 10;
    scoped_thread t(thread(func(some_local_state)));
}

int main()
{
    f();
    return 0;
}

