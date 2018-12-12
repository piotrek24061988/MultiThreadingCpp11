#include <iostream>
#include <algorithm>
#include <functional>
#include <numeric>
#include <thread>
#include <vector>
#include <list>
#include <future>
#include <exception>
#include <numeric>
using namespace std;

class join_threads //RAII to join created threads at the end.
{
    vector<thread> & threads;
public:
    explicit join_threads(vector<thread> & threads_) : threads(threads_) {}

    ~join_threads() {
        for(unsigned long i = 0; i < threads.size(); ++i) {
            if(threads[i].joinable()) {
                threads[i].join();
            }
        }
    }
};

template<typename Iterator>
void parallel_partial_sum(Iterator first, Iterator last)
{
    typedef typename Iterator::value_type value_type;

    struct process_chunk
    {                                                   //Future of last value from prev part of data.
        void operator()(Iterator begin, Iterator last, future<value_type> * previous_end_value,
                        promise<value_type> * end_value) {//Last value for current part of data.
            cout << "thread_id: " << this_thread::get_id() << endl;
            try {
                Iterator end = last;
                ++end;
                partial_sum(begin, end, begin);//partial_sum just for this part of data without prev value.
                if(previous_end_value) {//Is this firs part of data?
                    value_type addend = previous_end_value->get();//No. Wait for last value from prev part.
                    *last += addend;
                    if(end_value) {
                        end_value->set_value(*last);//First set end_value for next part of data
                    }                               //to inscrease parallel performence.
                    //Add previous_end_value to all elements from this parts of data
                    for_each(begin, last, [addend](value_type & item){item += addend;});
                } else if(end_value) {//This is first part of data. So just set end_value for
                    end_value->set_value(*last); //next part of data to increase parallel performance.
                }
            } catch(...) {
                if(end_value) {
                    end_value->set_exception(current_exception());
                } else {
                    throw;
                }
            }
        }
    };

    unsigned long const length = distance(first, last);

    if(!length) return;

    unsigned long const min_per_thread = 25;
    unsigned long const max_threads = (length + min_per_thread-1)/min_per_thread;

    unsigned long const hardware_threads = thread::hardware_concurrency();

    unsigned long const num_threads = min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
    unsigned long const block_size = length/num_threads;

    typedef typename Iterator::value_type value_type;
    vector<thread> threads(num_threads - 1);
    vector<promise<value_type>> end_values(num_threads-1);//Last elements for each part of data.
    vector<future<value_type>> previous_end_values;//Last element from previous part of data.
    previous_end_values.reserve(num_threads - 1);
    join_threads joiner(threads);

    Iterator block_start = first;
    //Iterate for every parts of elements.
    for(unsigned long i = 0; i < (num_threads - 1); ++i) {
        Iterator block_last = block_start;
        advance(block_last, block_size - 1);//Last element from curent parts of data.
        threads[i] = thread(process_chunk(), block_start, block_last, (i!=0) ? &previous_end_values[i-1]: 0, &end_values[i]);
        block_start = block_last;
        ++block_start;
        //Last value of this part of data to be used in next iteration.
        previous_end_values.push_back(end_values[i].get_future());
    }
    Iterator final_element = block_start;
    advance(final_element, distance(block_start, last) - 1);
    //Process last part of data elements in current thread.
    process_chunk()(block_start, final_element, (num_threads>1) ? &previous_end_values.back() : 0, 0);
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

    parallel_partial_sum(li.begin(), li.end());

    cout << "after" << endl;
    for(auto &a : li) cout << a << " ";
    cout << endl << endl;
}

