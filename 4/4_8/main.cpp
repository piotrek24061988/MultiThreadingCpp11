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
    if(input.empty()) {
        return input;
    }
    list<T> result;
    result.splice(result.begin(), input, input.begin());//Copy input list.
    T const & pivot = *result.begin();//First element of list is dividing point.

    //Input list diveded to values higher and lower than divide_point.
    auto divide_point = partition(input.begin(), input.end(), [&](const T & t){return t < pivot;});

    list<T> lower_part;//Get lower values sublist.
    lower_part.splice(lower_part.begin(), input, input.begin(), divide_point);

    auto new_lower(sequential_quick_sort(move(lower_part)));//Call Yourself for lowel values sublist.
    auto new_higher(sequential_quick_sort(move(input)));//Call Yourself for higher values sublist.

    result.splice(result.end(), new_higher);//Create result list form lower and higher sublists.
    result.splice(result.begin(), new_lower);
    return result;
}

template<typename T>
list<T> parallel_quick_sort(list<T> input)
{
    if(input.empty()) {
        return input;
    }
    list<T> result;
    result.splice(result.begin(), input, input.begin());//Copy input list.
    T const & pivot = *result.begin();//First element of list is dividing point.

    //Input list diveded to values higher and lower than divide_point.
    auto divide_point = partition(input.begin(), input.end(), [&](const T & t){return t < pivot;});

    list<T> lower_part;//Get lower values sublist;
    lower_part.splice(lower_part.begin(), input, input.begin(), divide_point);

    //auto new_lower(sequential_quick_sort(move(lower_part)));
    //auto new_higher(sequential_quick_sort(move(input)));
    //Call Yourself for lowel values sublist in separated thread.
    future<list<T>> new_lower(async(&parallel_quick_sort<T>, move(lower_part)));
    auto new_higher(parallel_quick_sort(move(input)));//Call Yourself for higher values sublist.

    result.splice(result.end(), new_higher);//Create result list form lower and higher sublists.
    result.splice(result.begin(), new_lower.get());
    return result;
}

int main()
{
    list<int> a {4, 3, 6, 2, 7, 5, 8, 9, 1};
    for(auto & x : a)
    {
        cout << x << " ";
    }
    cout << endl;

    for(auto&  x : sequential_quick_sort(a))
    {
        cout << x << " ";
    }
    cout << endl;

    for(auto & x : parallel_quick_sort(a))
    {
        cout << x <<  " ";
    }
    cout << endl;

    return 0;
}

