#include <iostream>
#include <algorithm>
#include <numeric>
#include <thread>
#include <vector>
#include <list>
#include <future>
using namespace std;

class join_threads {
    vector<thread> & threads;
public:
    explicit join_threads(vector<thread> & threads_) : threads(threads_){}

    ~join_threads() {
        for(unsigned long i = 0; i < threads.size(); ++i) {
            if(threads[i].joinable()) {
                threads[i].join();
            }
        }
    }
};

template<typename Iterator, typename Func>
void parallel_for_each(Iterator first, Iterator last, Func f) {
    unsigned long const length = distance(first, last);

    if(!length) return;

    unsigned long const min_per_thread = 25;
    unsigned long const max_threads = (length+min_per_thread-1)/min_per_thread;

    unsigned long const hardware_threads = thread::hardware_concurrency();

    unsigned long const num_threads = min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
    unsigned long const block_size = length / num_threads;

    vector<future<void>> futures(num_threads - 1);
    vector<thread> threads(num_threads - 1);
    join_threads joiner(threads);

    Iterator block_start = first;
    for(unsigned long i = 0; i < (num_threads - 1); ++i) {
            Iterator block_end = block_start;
            advance(block_end, block_size);
            std::packaged_task<void(void)> task([=]() {
                cout << "thread_id: " << this_thread::get_id() << endl;
                for_each(block_start, block_end, f);
            });
            futures[i] = task.get_future();
            threads[i] = thread(move(task));
            block_start = block_end;
    }

    for_each(block_start, last, f);
    for(unsigned long i = 0; i < (num_threads-1); ++i) {
        futures[i].get();
    }
}

int main()
{
    list<int> li = { 1,  2,  3,  4,  5,  6,  7,  8,  9, 10,
                    11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                    21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
                    31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
                    41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
                    51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
                    61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
                    71, 72, 73, 74, 75, 76, 77, 78, 79, 80,
                    81, 82, 83, 84, 85, 86, 87, 88, 89, 90,
                    91, 92, 93, 94, 95, 96, 97, 98, 99, 100};

    cout << "before" << endl;
    for(auto &a : li) cout << a << " ";
    cout << endl;

    parallel_for_each(li.begin(), li.end(), [](int & i){i *= 2;});

    cout << "after" << endl;
    for(auto &a : li) cout << a << " ";
    cout << endl;
}
