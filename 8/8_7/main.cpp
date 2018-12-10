#include <iostream>
#include <algorithm>
#include <numeric>
#include <thread>
#include <vector>
#include <list>
#include <future>
#include <iterator>
using namespace std;

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {

    unsigned long const length = distance(first, last);
    unsigned long const max_chunk_size = 25;

    if(length <= max_chunk_size) {
        return accumulate(first, last, init);
    } else {
        Iterator mid_point = first;
        advance(mid_point, length/2);
        future<T> first_half_result = async(parallel_accumulate<Iterator, T>, first, mid_point, init);
        T second_half_result = parallel_accumulate(mid_point, last, T());
        return first_half_result.get() + second_half_result;
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

    int sum;
    auto start = chrono::system_clock::now();
    sum = accumulate(li.begin(), li.end(), sum);
    cout << "time: " << chrono::duration<double>(chrono::system_clock::now() - start).count()  << endl;
    cout << "sequence accumaluate: " << sum  << endl;

    sum = 0;

    start = chrono::system_clock::now();
    sum = parallel_accumulate(li.begin(), li.end(), sum);
    cout << "time: " << chrono::duration<double>(chrono::system_clock::now() - start).count() << endl;
    cout << "parallel accumaluate: " << sum  << endl;
}
