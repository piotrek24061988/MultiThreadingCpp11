#include <iostream>
#include <algorithm>
#include <numeric>
#include <thread>
#include <vector>
#include <list>
#include <future>
using namespace std;

template<typename Iterator, typename Func>
void parallel_for_each(Iterator first, Iterator last, Func f) {
    unsigned long const length = distance(first, last);
    if(!length) return;

    unsigned long const max_chunk_size = 25;
    if(length <= (2 * max_chunk_size)) {
        for_each(first, last, f);
    } else {
        Iterator mid_point = first;
        std::advance(mid_point, length / 2);
        future<void> first_half = async(parallel_for_each<Iterator, Func>, first, mid_point, f);
        parallel_for_each(mid_point, last, f);
        first_half.get();
    }
}

int main()
{
    list<int> li = {  1, 2,  3,  4,  5,  6,  7,  8,  9, 10,
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

    parallel_for_each(li.begin(), li.end(), [](int & i){i *= 3;});

    cout << "after" << endl;
    for(auto &a : li) cout << a << " ";
    cout << endl;
}
