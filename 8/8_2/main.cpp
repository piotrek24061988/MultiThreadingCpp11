#include <iostream>
#include <thread>
#include <mutex>
#include <future>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <vector>
#include <list>
#include <stack>
#include <algorithm>
using namespace std;

template <typename T>
class threadsafe_stack
{
private:
    stack<T> data;
    mutable mutex m;
    condition_variable data_cond;
public:
    threadsafe_stack(){}
    threadsafe_stack(const threadsafe_stack & other) = delete;
    threadsafe_stack & operator=(const threadsafe_stack & other) = delete;

    void push(T new_value) {
        {
            lock_guard<mutex> lock(m);
            data.push(move(new_value));
            data_cond.notify_one();
        }
    }

    shared_ptr<T> pop() {
        shared_ptr<T> res;
        {
            unique_lock<mutex> lock(m);
            if(data_cond.wait_for(lock, 100ms, [this]{return !data.empty();})) {
                res = make_shared<T>(move(data.top()));
                data.pop();
            } else {
                res = make_shared<T>();
            }
        }
        return res;
    }

    void pop(T & value) {
        unique_lock<mutex> lock(m);
        if(data_cond.wait_for(lock, 100ms, [this]{return !data.empty();})) {
            value = move(data.top());
            data.pop();
        }
    }
};

template<typename T>
struct sorter {
    struct chunk_to_sort { //Smallest part of data which would be sorted in one thread.
        list<T> data;
        promise<list<T>> promises;
    };

    threadsafe_stack<chunk_to_sort> chunks;//Unsorted groups of data.
    vector<thread> threads;

    unsigned const max_thread_count;
    atomic<bool> end_of_data;

    sorter(): max_thread_count(thread::hardware_concurrency() - 1), end_of_data(false) {}

    ~sorter() {
        end_of_data = true;
        for(unsigned i = 0; i < threads.size(); ++i) threads[i].join();
    }

    void try_sort_chunk() {
        shared_ptr<chunk_to_sort> chunk = chunks.pop();
        if(chunk) { //If there is something on the list to be sorted
            sort_chunk(chunk); //sort it.
        }
    }

    list<T> do_sort(list<T> & chunk_data) {
        if(chunk_data.empty()) {
            return chunk_data;
        }

        list<T> result;
        result.splice(result.begin(), chunk_data, chunk_data.begin());//splice/transfer elements from one list to another
        T const & partition_val = *result.begin();//First element is dividing point.

        typename list<T>::iterator divide_point = //divide list to 2 sublists, first passing lambda and second not passing lambda condition
                partition(chunk_data.begin(), chunk_data.end(), [&](T const & val){return val < partition_val;});

        chunk_to_sort new_lower_chunk; //first sublist
        new_lower_chunk.data.splice(new_lower_chunk.data.end(), chunk_data, chunk_data.begin(), divide_point);

        future<list<T>> new_lower = new_lower_chunk.promises.get_future();
        chunks.push(move(new_lower_chunk));//push first sublist to stack holding sublists and take asigned future.

        if(threads.size() < max_thread_count) {//If we have available thread in threads pool.
            threads.push_back(thread(&sorter<T>::sort_thread, this));//create thread with sort_thread function from this class
        }

        list<T> new_higher(do_sort(chunk_data));//call revursively the same function for second sublist

        result.splice(result.end(), new_higher);
        while(new_lower.wait_for(chrono::seconds(0)) != future_status::ready) {
            try_sort_chunk();//try to sort one sublist from the stack
        }

        result.splice(result.begin(), new_lower.get());//insert sorted list to results list
        return result;
    }

    void sort_chunk(shared_ptr<chunk_to_sort> const & chunk) {
        chunk->promises.set_value(do_sort(chunk->data));
    }

    void sort_thread() {
        cout << "Thread is: " << this_thread::get_id() << endl;
        while(!end_of_data) {
            try_sort_chunk();
            this_thread::yield();
        }
    }
};

template<typename T>
list<T> parallel_quick_sort(list<T> input) {
    if(input.empty()) {
        return input;
    }
    sorter<T> s;
    return s.do_sort(input);
}

int main()
{
    list<int> l { 300, 1, 34, 424, 56, 65, 2, 556, 54, 78, 45, 87, 79, 88, 99, 5556, 564, 7, 8, 0, 678, 242, 465, 876, 43, 655};

    for(auto & a : l) {
        cout << a << " ";
    }
    cout << endl;

    l = parallel_quick_sort(l);

    for(auto & a : l) {
        cout << a << " ";
    }
    cout << endl;
}
