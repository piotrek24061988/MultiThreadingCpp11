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

class barrier
{
    atomic<unsigned> count;//Total number of threads
    atomic<unsigned> spaces;//Free places for threads
    atomic<unsigned> generation;

public:
    explicit barrier(unsigned count_) : count(count_), spaces(count_), generation(0){}

    void wait() {
        unsigned const my_generation = generation;
        if(!--spaces) {//Adding new waiting thread decrease number of free places.
            spaces = count.load();//If no more pleaces reasign initial value and
            ++generation;//let know waiting threads that they can continue they work.
        } else {
            while(generation==my_generation) {//If barier achieved, stop waiting.
                this_thread::yield();//Do not waiste processor time when waiting.
            }
        }
    }

    void done_waiting() {//Decrease total number of waiting threads to use
        --count;         //new lower value when current barier achieved.
        if(!--spaces) {//Decrease number of free places.
            spaces=count.load();//If no more pleaces reasign initial value and
            ++generation;//let know waiting threads that they can continue they work.
        }
    }
};

//example of internal work
//[0, 1, 2, 3, 4, 5]
//thread 1 [0, x1+0, x2+0, x3+0, x4+0, x5+0]
//thread 2 [0,    1, x2+1, x3+1, x4+1, x5+1]
//thread 3 [0,    1,    3, x3+2, x4+2, x5+2]
//thread 4 [0,    1,    3,    6, x4+3, x5+3]
//thread 5 [0,    1,    3,    6,   10, x5+4]
//thread 6 [0,    1,    3,    6,   20, 15]
template <typename Iterator>
void parallel_partial_sum(Iterator first, Iterator last) {
    typedef typename Iterator::value_type value_type;

    struct process_element {
        void operator()(Iterator first, Iterator last, vector<value_type> & buffer,
                        unsigned i, barrier & b) {

            Iterator temp = first;
            advance(temp, i);
            value_type & ith_element = *temp;
            bool update_source = false;

            for(unsigned step = 0, stride = 1; stride <= i; ++step, stride *= 2) {
                temp = first;
                advance(temp, i - stride);
                value_type const & source = (step % 2) ? buffer[i] : ith_element;
                value_type & dest = (step % 2) ? ith_element : buffer[i];
                value_type const & addend = (step % 2) ? buffer[i - stride] : *temp;

                dest = source + addend;
                update_source = !(step % 2);

                b.wait();
            }
            //These 2 lines are just for debug
            cout << "thread_id: " << this_thread::get_id() << endl;
            for(auto i = first; i != last; i++) { cout << *i << " ";} cout << endl;
            if(update_source) {
                ith_element = buffer[i];
            }
            b.done_waiting();
        }
    };

    unsigned long const length = distance(first, last);

    if(length <= 1) return;

    vector<value_type> buffer(length);
    barrier b(length);

    vector<thread> threads(length - 1);//Number of threads depends on number of data
    join_threads joiner(threads);

    Iterator block_start = first;
    for(unsigned long i = 0; i < (length - 1); ++i) {
        threads[i] = thread(process_element(), first, last, ref(buffer), i, ref(b));
    }
    process_element()(first, last, buffer, length - 1, b);
}

int main()
{
    list<int> li = { 0, 1,  2,  3,  4,  5,  6,  7,  8,  9, 10,
                       11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                       21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
                       31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
                       41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
                       51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
                       61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
                       71, 72, 73, 74, 75, 76, 77, 78, 79, 80,
                       81, 82, 83, 84, 85, 86, 87, 88, 89, 90,
                       91, 92, 93, 94, 95, 96, 97, 98, 99, 100};
    //list<int> li = { 0, 1,  2,  3,  4,  5};

    cout << "before" << endl;
    for(auto &a : li) cout << a << " ";
    cout << endl;

    parallel_partial_sum(li.begin(), li.end());

    cout << "after" << endl;
    for(auto &a : li) cout << a << " ";
    cout << endl << endl;
}

