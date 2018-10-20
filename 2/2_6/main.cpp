#include <iostream>
#include <thread>
#include <algorithm>
#include <vector>
#include <numeric>
#include <chrono>
using namespace std;

template <typename Iterator, typename T>
struct accumulate_block
{
    void operator()(Iterator first, Iterator last, T & result)
    {
        result = accumulate(first, last, result);
        cout << "thread id: " << this_thread::get_id() << endl;
    }
};

template <typename Iterator, typename T>
T parralel_accumulate(Iterator first, Iterator last, T init)
{
    const unsigned long length = distance(first, last);

    if(!length)
    {
        return init;
    }

    constexpr unsigned long min_per_thread = 25;
    const unsigned long max_threads = (length + min_per_thread - 1)/min_per_thread;

    const unsigned long hardware_threads = thread::hardware_concurrency();

    const unsigned long num_threads = min(hardware_threads != 0 ? hardware_threads : 2, max_threads);

    const unsigned long block_size = length / num_threads;

    vector<T> results(num_threads);
    vector<thread> threads(num_threads - 1);

    Iterator block_start = first;
    for(unsigned long i = 0; i < (num_threads - 1); i++)
    {
        Iterator block_end = block_start;
        advance(block_end, block_size);
        threads[i] = thread(accumulate_block<Iterator, T>(), block_start, block_end, ref(results[i]));
        block_start = block_end;
    }

    accumulate_block<Iterator, T>()(block_start, last, results[num_threads-1]);

    for_each(threads.begin(), threads.end(), mem_fn(&thread::join));

    return accumulate(results.begin(), results.end(), init);
}

int main()
{
    std::vector<int> vec= {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                           1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                           1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                           1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                           1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                           1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                           1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                           1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                           1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                           1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                           1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                           1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                           1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25};

    int a = 0;
    auto start = chrono::system_clock::now();
    cout << "thread id: " << this_thread::get_id() << endl;
    a =  accumulate(vec.begin(), vec.end(), a);
    cout << "val: "<< a << ", time: " << chrono::duration<double>(chrono::system_clock::now() - start).count()  << endl;

    a = 0;

    start = chrono::system_clock::now();
    a = parralel_accumulate(vec.begin(), vec.end(), a);
    cout << "val: "<< a << ", time: " << chrono::duration<double>(chrono::system_clock::now() - start).count() << endl;

    return 0;
}

