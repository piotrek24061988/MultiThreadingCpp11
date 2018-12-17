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

    //Take one task from tasks queue and execute it in current thread.
    void run_pending_task() {
        function_wrapper task;
        if(work_queue.try_pop(task)) {
            task();
        } else {
            this_thread::yield();
        }
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

template<typename T>
struct sorter {
private:
    thread_pool pool;
public:
    list<T> do_sort(list<T> & chunk_data) {
        if(chunk_data.empty()) return chunk_data;//If nothing to sort.

        list<T> result;
        result.splice(result.begin(), chunk_data, chunk_data.begin());//Copy data to result.

        T const & partition_val = *result.begin();//First element is partition value.
        //order list into two sublists, for first part passing lambda and second part not passing labda.
        typename list<T>::iterator divide_point = partition(chunk_data.begin(), chunk_data.end(), [&](T const & val){return val < partition_val;});

        list<T> new_lower_chunk;//Take first sublist
        new_lower_chunk.splice(new_lower_chunk.begin(), chunk_data, chunk_data.begin(), divide_point);

        //Add recurency this function to worker thread with one sublist and get it future.
        future<list<T>> new_lower = pool.submit(bind(&sorter::do_sort, this, move(new_lower_chunk)));
        list<T> new_higher(do_sort(chunk_data));//Call requrency this function with second sublist.

        result.splice(result.end(), new_higher);//Cummulate result from current thread sublist.
        while(new_lower.wait_for(chrono::seconds(0)) == future_status::timeout)
        {   //While task not done try to do it in current thread.
            pool.run_pending_task();
        }
        //Cummulate result from separated thread sublist and return.
        result.splice(result.begin(), new_lower.get());
        return result;
    }
};

template<typename T>
list<T> parallel_quick_sort(list<T> input) {
    if(input.empty()) return input;

    sorter<T> s;
    return s.do_sort(input);
}

int main()
{
    list<int> l { 99, 77, 1, 34, 424, 56, 65, 2, 5, 3, 33, 46, 53, 64, 87, 98, 556, 54, 78, 45, 87, 79, 88, 5556,
                  564, 7, 8, 0, 678, 242, 465, 876, 43, 655, 5544, 766, 777, 800};

    l = parallel_quick_sort(l);

    for(auto & a : l) {
        cout << a << " ";
    }
    cout << endl;
}
