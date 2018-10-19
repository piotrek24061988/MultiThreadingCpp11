#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
using namespace std;

vector<int> queue_data;
atomic<int> count;
atomic<bool> endloop;

void populate_queue()
{
    endloop.store(false);
    for(unsigned j = 0; j < 10; j++)
    {
        unsigned const number_of_items = 20 - j;
        queue_data.clear();
        for(unsigned i = 0; i < number_of_items; i++)
        {
            queue_data.push_back(i);
        }

        count.store(number_of_items, memory_order_release);
        cout << "populate_queue(): " << j << endl;
        this_thread::sleep_for(10s);
    }
    endloop.store(true);
}

void consume_queue_items(int i)
{
    while(!endloop.load())
    {
        int item_index;
        if((item_index = count.fetch_sub(1, memory_order_acquire)) <= 0)
        {
            cout << "populate_queue(" << i << ") wait for more" << endl;
            this_thread::sleep_for(5s);
            continue;
        }
        this_thread::sleep_for(1s);
        cout << "populate_queue(" << i << "):" <<  queue_data[item_index - 1] << endl;
    }
}

int main()
{
    thread a(populate_queue);
    thread b(consume_queue_items, 1);
    thread c(consume_queue_items, 2);
    a.join();
    b.join();
    c.join();

    return 0;
}

