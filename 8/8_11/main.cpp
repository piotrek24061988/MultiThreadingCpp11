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

template<typename Iterator, typename MatchType>
Iterator parralel_find_impl(Iterator first, Iterator last, MatchType match, atomic<bool> & done)
{
    try {
        unsigned long const length = distance(first, last);
        unsigned long const min_per_thread = 25;
        //If container length is  to small to divide it beetween threads
        if(length < ( 2 * min_per_thread)) {
            //search for all elements in current groups until another thread
            for(; (first != last) && !done.load(); ++first) {//did not find result.
                if(*first==match) {//Or tihs thread found result.
                    done.store(true);//So notify other threads.
                    return first;
                }
            }
            return last;//If element not founded return las position of group
        //Divide container elements to groups for dedicated threads
        } else {
            Iterator mid_point = first;
            advance(mid_point, length / 2);
            //Do first half recursively in separated thread.
            future<Iterator> async_result = async(&parralel_find_impl<Iterator, MatchType>, mid_point, last, match, ref(done));
            //Do second half recursively in cerrent thread.
            Iterator const direct_result = parralel_find_impl(first, mid_point, match, done);
            //If not founded in current thread try to get results from other threads.
            //If they also doesnt have result last is returned. Otherwise founded
            //element position is returned.
            return (direct_result==mid_point) ? async_result.get() : direct_result;
        }
    } catch(...) {
        done = true;
        throw;
    }
}

template<typename Iterator, typename MatchType>
Iterator parallel_find(Iterator first, Iterator last, MatchType match) {
    atomic<bool> done(false);
    return parralel_find_impl(first, last, match, done);
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

    auto it = parallel_find(li.begin(), li.end(), 68);
    li.erase(it);

    cout << "after" << endl;
    for(auto &a : li) cout << a << " ";
    cout << endl << endl;
}

