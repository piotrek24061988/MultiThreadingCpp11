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
            if(threads[i].joinable()) threads[i].join();
        }
    }
};

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {

    unsigned long const length = distance(first, last);

    if(!length) {
        return init;
    }

    unsigned long const min_per_thread = 25;
    unsigned long const max_threads = (length+min_per_thread-1)/min_per_thread;
    unsigned long const hardware_threads = thread::hardware_concurrency();
    unsigned long const num_threads = min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
    unsigned long const block_size = length / num_threads;

    vector<future<T>> futures(num_threads - 1);
    vector<thread> threads(num_threads - 1);
    join_threads joiner(threads);
    auto accumulate_block = [](Iterator first, Iterator last) {
        cout << "thread_id: " << this_thread::get_id() << endl;
        return std::accumulate(first, last, T());
    };

    Iterator block_start = first;
    for(unsigned long i = 0; i < (num_threads - 1); ++i) {
            Iterator block_end = block_start;
            advance(block_end, block_size);
            std::packaged_task<T(Iterator, Iterator)> task(accumulate_block);
            futures[i] = task.get_future();
            threads[i] = thread(move(task), block_start, block_end);
            block_start = block_end;
    }
    T last_result = accumulate_block(block_start, last);
    T result = init;
    for(unsigned long i = 0; i < (num_threads - 1); ++i) {
        result += futures[i].get();
    }
    return (result += last_result);
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

    int sum;
    auto start = chrono::system_clock::now();
    //Sequential sum of elements by std library
    sum = accumulate(li.begin(), li.end(), sum);
    cout << "time: " << chrono::duration<double>(chrono::system_clock::now() - start).count()  << endl;
    cout << "sequence accumaluate: " << sum  << endl;

    sum = 0;

    start = chrono::system_clock::now();
    //Parallel sum of elements by own parallel_function
    sum = parallel_accumulate(li.begin(), li.end(), sum);
    cout << "time: " << chrono::duration<double>(chrono::system_clock::now() - start).count() << endl;
    cout << "parallel accumaluate: " << sum  << endl;
}
