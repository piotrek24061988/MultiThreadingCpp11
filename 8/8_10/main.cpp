#include <iostream>
#include <algorithm>
#include <functional>
#include <numeric>
#include <thread>
#include <vector>
#include <list>
#include <future>
#include <exception>
using namespace std;

class join_threads
{
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

template<typename Iterator, typename MatchType>
Iterator parallel_find(Iterator first, Iterator last, MatchType match)
{
    struct find_element
    {
        void operator()(Iterator begin, Iterator end, MatchType match,
                        promise<Iterator> * result, atomic<bool> * done_flag) {
            try {
                //Continue searching element by element or stop if another thread
                //already found value for his part of elements
                for(; (begin != end) && !done_flag->load(); ++begin) {
                    if(*begin==match) {//If this thread found value
                        result->set_value(begin);//save it place
                        done_flag->store(true);//and notify other threads
                        return;
                    }
                }
            } catch(...) {//If there is any exception we save it in promise
                try {     //and finish other threads
                    result->set_exception(current_exception());
                    done_flag->store(true);

                } catch(...) {}//Ignore any exception on this level
            }
        }
    };

    unsigned long const length = distance(first, last);
    if(!length) return last;

    unsigned long const min_per_thread = 25;
    unsigned long const max_threads = (length + min_per_thread-1)/min_per_thread;
    unsigned long const hardware_threads = thread::hardware_concurrency();
    unsigned long const num_threads = min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
    unsigned long const block_size = length/num_threads;

    promise<Iterator> result;
    atomic<bool> done_flag(false);
    vector<thread> threads(num_threads - 1);
    {
        join_threads joiner(threads);

        Iterator block_start = first;
        for(unsigned long i = 0; i < (num_threads-1); ++i) {
            Iterator block_end = block_start;
            advance(block_end, block_size);
            threads[i] = thread(find_element(), block_start, block_end, match, &result, &done_flag);
            block_start = block_end;
        }
        find_element()(block_start, last, match, &result, &done_flag);
    }
    if(!done_flag.load()) {//Searched element was not founded
        return last;
    }
    return result.get_future().get();//Provide searched element or exception
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
    cout << endl << endl;

    auto it = parallel_find(li.begin(), li.end(), 68);
    li.erase(it);

    cout << "after" << endl;
    for(auto &a : li) cout << a << " ";
    cout << endl << endl;
}

