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
#include <mutex>
#include <atomic>
#include <chrono>
#include <exception>
#include <queue>
#include <condition_variable>
using namespace std;

//Objects of class std::packaged_task<> cant be copied. Can be only moved.
//So std::function<> require copy constructors object. So it need to be
//replaced to function_wrapper which would be ok for std::packaged_task<>
class function_wrapper {
    struct impl_base {
        virtual void call() = 0;
        virtual ~impl_base(){}
    };

    unique_ptr<impl_base> impl;

    template<typename F>
    struct impl_type : impl_base {
        F f;
        impl_type(F && f_) : f(move(f_)){}
        void call() {
            f();
        }
    };

public:
    template<typename F>
    function_wrapper(F && f): impl(new impl_type<F>(move(f))) {}

    void operator()() {
        impl->call();
    }

    //Move semantics allowed.
    function_wrapper() = default;
    function_wrapper(function_wrapper && other) : impl(move(other.impl)) {}
    function_wrapper & operator=(function_wrapper && other) {
        impl = move(other.impl);
        return *this;
    }

    //Copy semantics not allowed.
    function_wrapper(const function_wrapper &) = delete;
    function_wrapper(function_wrapper&) = delete;
    function_wrapper & operator=(const function_wrapper &) = delete;
};

template<typename T>
class threadsafe_queue {
private:
    mutable mutex mut;
    queue<shared_ptr<T>> data_queue;
    condition_variable data_cond;
public:
    threadsafe_queue(){}

    void push(T new_value) {
        shared_ptr<T> data = make_shared<T>(move(new_value));
        lock_guard<mutex> lk(mut);
        data_queue.push(data);
        data_cond.notify_one();
    }

    void wait_and_pop(T & value) {
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [this](){return !data_queue.empty();});
        value = move(*data_queue.front());
        data_queue.pop();
    }

    shared_ptr<T> wait_and_pop() {
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [this](){return !data_queue.empty();});
        shared_ptr<T> res = data_queue.front();
        data_queue.pop();
        return res;
    }

    bool try_pop(T & value) {
        lock_guard<mutex> lk(mut);
        if(data_queue.empty()) {
            return false;
        }
        value = move(*data_queue.front());
        data_queue.pop();
        return true;
    }

    shared_ptr<T> try_pop() {
        lock_guard<mutex> lk(mut);
        if(data_queue.empty()) {
            return shared_ptr<T>();
        }
        shared_ptr<T> res = data_queue.front();
        data_queue.pop();
        return res;
    }

    bool empty() const {
        lock_guard<mutex> lk(mut);
        return data_queue.empty();
    }
};

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

class thread_pool
{
    atomic_bool done;
    //Own function_wrapper for tasks to be done in thread_pool instead of
    //std::function<> to accept std::packaged_task<>.
    threadsafe_queue<function_wrapper> work_queue;
    vector<thread> threads;
    join_threads joiner;

    void worker_thread()
    {
        cout << "thread_id: " << this_thread::get_id() << endl;
        while(!done) {
            function_wrapper task;
            if(work_queue.try_pop(task)) {
                task();
            } else {
                this_thread::yield();
            }
        }
    }

public:
    thread_pool() : done(false), joiner(threads) {
        unsigned const thread_count = thread::hardware_concurrency();

        try {
            for(unsigned i = 0; i < thread_count; i++) {
                threads.push_back(thread(&thread_pool::worker_thread, this));
            }
        } catch(...) {
            done = true;
            throw;
        }
    }

    ~thread_pool() {
        done = true;
    }

    //Submit task to pool thread and get future associated with the result
    //of this task.
    template<typename FunctionType>
    future<typename result_of<FunctionType()>::type> submit(FunctionType f) {
        typedef typename result_of<FunctionType()>::type result_type;
        //Create packaged_task based on submited function. Wrap this function.
        packaged_task<result_type()> task(move(f));
        //Get future asociated with the result of this task.
        future<result_type> res(task.get_future());
        work_queue.push(move(task));//Move task to queue.
        return res;//Return result.
    }
};

template <typename Iterator, typename T>
struct accumulate_block {
    Iterator first, last;
    T result;
    accumulate_block(Iterator first_, Iterator last_, T & result_)
        : first(first_), last(last_), result(result_) {}

    accumulate_block() : result() {}

    T & operator()(Iterator first_, Iterator last_, T & result_) {
        result_ = accumulate(first_, last_, result_);
        return result_;
    }

    T operator()() {
        result = accumulate(first, last, result);
        return result;
    }
};

//Parrallel_accumulate using threads worker.
template<typename Iterator, typename T>
T parrallel_accumulate(Iterator first, Iterator last, T init) {
    unsigned long const length = distance(first, last);

    if(!length) {
        return init;
    }

    unsigned long const block_size = 25;
    unsigned long const num_blocks = (length + block_size - 1) / block_size;

    vector<future<T>> futures(num_blocks - 1);//Number of parallel results from background threads.
    thread_pool pool;

    Iterator block_start = first;
    for(unsigned long i = 0; i < (num_blocks - 1); ++i) {
        Iterator block_end = block_start;
        advance(block_end, block_size);
        T tmp_init = 0;
        futures[i] = pool.submit(accumulate_block<Iterator, T>(block_start, block_end, tmp_init));
        block_start = block_end;
    }
    T tmp_init = 0;
    T last_result = accumulate_block<Iterator, T>()(block_start, last, tmp_init);//Result from current thread.
    //Sum all subresults and return.
    T result = init;
    for(unsigned long i = 0; i < (num_blocks - 1); ++i) {
        result += futures[i].get();
    }
    result += last_result;
    return result;
}

int main()
{
    std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
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
    a =  accumulate(vec.begin(), vec.end(), a);
    cout << "val: "<< a << ", time: " << chrono::duration<double>(chrono::system_clock::now() - start).count()  << endl;

    a = 0;

    start = chrono::system_clock::now();
    a = parrallel_accumulate(vec.begin(), vec.end(), a);
    cout << "val: "<< a << ", time: " << chrono::duration<double>(chrono::system_clock::now() - start).count() << endl;
}
