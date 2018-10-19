#include <iostream>
#include <atomic>
#include <thread>
using namespace std;

atomic<int> x{0}, y{0}, z{0};
atomic<bool> go{false};

constexpr unsigned int loop_count = 10;

struct read_values
{
    int x, y, z;
};

read_values values1[loop_count];
read_values values2[loop_count];
read_values values3[loop_count];
read_values values4[loop_count];
read_values values5[loop_count];

void increment(atomic<int> * var_to_inc, read_values * values)
{
    while(!go)
    {
        this_thread::yield();
    }

    for(unsigned int i = 0; i < loop_count; i++)
    {
        values[i].x = x.load(memory_order_relaxed);
        values[i].y = y.load(memory_order_relaxed);
        values[i].z = z.load(memory_order_relaxed);
        var_to_inc->store(i + 1, memory_order_relaxed);
        this_thread::yield();
    }
}

void read_vals(read_values * values)
{
    while(!go)
    {
        this_thread::yield();
    }

    for(unsigned int i = 0; i < loop_count; i++)
    {
        values[i].x = x.load(memory_order_relaxed);
        values[i].y = y.load(memory_order_relaxed);
        values[i].z = z.load(memory_order_relaxed);
        this_thread::yield();
    }
}

void print(read_values * v)
{
    for(unsigned int i = 0; i < loop_count; i++)
    {
        if(i)
        {
            cout << ", ";
        }
        cout << "(" << v[i].x << ", " << v[i].y << ", " << v[i].z << ")";
    }
    cout << endl;
}

int main()
{
    thread t1(increment, &x, values1);
    thread t2(increment, &y, values2);
    thread t3(increment, &z, values3);
    thread t4(read_vals, values4);
    thread t5(read_vals, values5);

    go = true;

    t5.join();
    t4.join();
    t3.join();
    t2.join();
    t1.join();

    print(values1);
    print(values2);
    print(values3);
    print(values4);
    print(values5);

    return 0;
}

