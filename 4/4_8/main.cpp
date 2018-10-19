#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <utility>
#include <future>
#include <list>
#include <algorithm>
using namespace std;

template<typename T>
list<T> sequential_quick_sort(list<T> input)
{
    if(input.empty())
    {
        return input;
    }
    list<T> result;
    result.splice(result.begin(), input, input.begin());
    T const & pivot = *result.begin();

    auto divide_point = partition(input.begin(), input.end(), [&](const T & t){return t < pivot;});

    list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);

    auto new_lower(sequential_quick_sort(move(lower_part)));
    auto new_higher(sequential_quick_sort(move(input)));

    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower);
    return result;
}

template<typename T>
list<T> parallel_quick_sort(list<T> input)
{
    if(input.empty())
    {
        return input;
    }
    list<T> result;
    result.splice(result.begin(), input, input.begin());
    T const & pivot = *result.begin();

    auto divide_point = partition(input.begin(), input.end(), [&](const T & t){return t < pivot;});

    list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);

    //auto new_lower(sequential_quick_sort(move(lower_part)));
    //auto new_higher(sequential_quick_sort(move(input)));
    future<list<T>> new_lower(async(&parallel_quick_sort<T>, move(lower_part)));
    auto new_higher(parallel_quick_sort(move(input)));

    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower.get() );
    return result;
}

int main()
{
    list<int> a {3, 6, 7, 5, 8, 9, 1};
    for(auto x : sequential_quick_sort(a))
    {
        cout << x << endl;
    }

    list<int> b {3, 6, 7, 5, 8, 9, 1};
    for(auto x : parallel_quick_sort(b))
    {
        cout << x << endl;
    }

    return 0;
}

