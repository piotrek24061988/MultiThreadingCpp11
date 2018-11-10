#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
using namespace std;

struct ThreadData
{
    vector<int> data;
    atomic<bool> data_ready {false};
};

void reader_thread(ThreadData & td) {
    for(int i = 1; i <= 20; i++) {
        while(!td.data_ready.load());
        cout << "reader_thread: " << td.data.back() << endl;
        td.data.pop_back();
        if(td.data.empty()) {
            td.data_ready = false;
        }
    }
}

void writer_thread(ThreadData & td) {
    for(auto & i : {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20}) {
        while(td.data_ready.load());
        cout << "writer_thread: " << i << endl;
        td.data.push_back(i);
        td.data_ready = true;
    }
}

int main()
{
    ThreadData td;

    thread t1(reader_thread, ref(td));
    thread t2(writer_thread, ref(td));
    t1.join();
    t2.join();

    return 0;
}

