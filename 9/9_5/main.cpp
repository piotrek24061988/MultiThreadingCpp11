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

class function_wrapper
{
    struct impl_base
    {
        virtual void call() = 0;
        virtual ~impl_base(){}
    };

    unique_ptr<impl_base> impl;

    template<typename F>
    struct impl_type : impl_base
    {
        F f;
        impl_type(F && f_) : f(move(f_)){}
        void call()
        {
            f();
        }
    };

public:
    template<typename F>
    function_wrapper(F && f):
        impl(new impl_type<F>(move(f)))
    {}

    void operator()()
    {
        impl->call();
    }

    function_wrapper() = default;

    function_wrapper(function_wrapper && other) : impl(move(other.impl))
    {}

    function_wrapper & operator=(function_wrapper && other)
    {
        impl = move(other.impl);
        return *this;
    }

    function_wrapper(const function_wrapper &) = delete;
    function_wrapper(function_wrapper&) = delete;
    function_wrapper & operator=(const function_wrapper &) = delete;
};

template<typename T>
class threadsafe_queue
{
private:
    mutable mutex mut;
    queue<shared_ptr<T>> data_queue;
    condition_variable data_cond;
public:
    threadsafe_queue(){}

    void push(T new_value)
    {
        shared_ptr<T> data(make_shared<T>(move(new_value)));
        lock_guard<mutex> lk(mut);
        data_queue.push(data);
        data_cond.notify_one();
    }

    void wait_and_pop(T & value)
    {
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [this](){return !data_queue.empty();});
        value = move(*data_queue.front());
        data_queue.pop();
    }

    shared_ptr<T> wait_and_pop()
    {
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [this](){return !data_queue.empty();});
        shared_ptr<T> res = data_queue.front();
        data_queue.pop();
        return res;
    }

    bool try_pop(T & value)
    {
        lock_guard<mutex> lk(mut);
        if(data_queue.empty())
        {
            return false;
        }
        value = move(*data_queue.front());
        data_queue.pop();
        return true;
    }

    shared_ptr<T> try_pop()
    {
        lock_guard<mutex> lk(mut);
        if(data_queue.empty())
        {
            return false;
        }
        shared_ptr<T> res = data_queue.front();
        data_queue.pop();
        return res;
    }

    bool empty() const
    {
        lock_guard<mutex> lk(mut);
        return data_queue.empty();
    }
};

class join_threads
{
    vector<thread> & threads;
public:
    explicit join_threads(vector<thread> & threads_) : threads(threads_){}
    ~join_threads()
    {
        for(unsigned long i = 0; i < threads.size(); ++i)
        {
            if(threads[i].joinable())
            {
                threads[i].join();
            }
        }
    }
};

class thread_pool
{
    atomic_bool done;
    vector<thread> threads;
    join_threads joiner;

    threadsafe_queue<function_wrapper> pool_work_queue;
    typedef queue<function_wrapper> local_queue_type;
    static thread_local unique_ptr<local_queue_type> local_work_queue;

    void worker_thread()
    {
        local_work_queue.reset(new local_queue_type);
        while(!done)
        {
            run_pending_task();
        }
    }

public:
    template<typename FunctionType>
    future<typename result_of<FunctionType()>::type> submit(FunctionType f)
    {
        typedef typename result_of<FunctionType()>::type result_type;

        packaged_task<result_type()> task(move(f));
        future<result_type> res(task.get_future());
        if(local_work_queue)
        {
            local_work_queue->push(move(task));
        }
        else
        {
            pool_work_queue.push(move(task));
        }
        return res;
    }


    void run_pending_task()
    {
        function_wrapper task;
        if(local_work_queue && !local_work_queue->empty())
        {
            task = move(local_work_queue->front());
            local_work_queue->pop();
            task();
        }
        else if(pool_work_queue.try_pop(task))
        {
            task();
        }
        else
        {
            this_thread::yield();
        }
    }

    thread_pool()
        : done(false), joiner(threads)
    {
        unsigned const thread_count = thread::hardware_concurrency();

        try
        {
            for(unsigned i = 0; i < thread_count; i++)
            {
                threads.push_back(thread(&thread_pool::worker_thread, this));
            }
        }
        catch(...)
        {
            done = true;
            throw;
        }
    }

    ~thread_pool()
    {
        done = true;
    }
};

thread_local unique_ptr<thread_pool::local_queue_type> thread_pool::local_work_queue;

template<typename T>
struct sorter
{
    thread_pool pool;

    list<T> do_sort(list<T> & chunk_data)
    {
        if(chunk_data.empty())
        {
            return chunk_data;
        }

        list<T> result;
        result.splice(result.begin(), chunk_data, chunk_data.begin());
        T const & partition_val = *result.begin();

        typename list<T>::iterator divide_point = partition(chunk_data.begin(), chunk_data.end(), [&](T const & val){return val < partition_val;});

        list<T> new_lower_chunk;
        new_lower_chunk.splice(new_lower_chunk.end(), chunk_data, chunk_data.begin(), divide_point);

        future<list<T>> new_lower = pool.submit(bind(&sorter::do_sort, this, move(new_lower_chunk)));
        list<T> new_higher(do_sort(chunk_data));

        result.splice(result.end(), new_higher);
        while(new_lower.wait_for(chrono::seconds(0)) == future_status::timeout)
        {
            pool.run_pending_task();
        }

        result.splice(result.begin(), new_lower.get());
        return result;
    }
};

template<typename T>
list<T> parallel_quick_sort(list<T> input)
{
    if(input.empty())
    {
        return input;
    }
    sorter<T> s;
    return s.do_sort(input);
}

int main()
{
    list<int> l { 1, 34, 424, 56, 65, 2, 5, 3, 33, 46, 53, 64, 77, 87, 98, 556, 54, 78, 45, 87, 79, 88, 99, 5556,
                  564, 7, 8, 0, 678, 242, 465, 876, 43, 655, 5544, 766, 777, 800};
    l = parallel_quick_sort(l);

    for(auto & a : l)
    {
        cout << a << " ";
    }
    cout << endl;
}
