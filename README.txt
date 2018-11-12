----------------------------------------------------------------------------------
Multithreading tutorial in Cpp11

Theory:
----------------------------------------------------------------------------------
thread t1(foo);

t1.join(); //Wait for thread to finish.
t1.detach(); //Run thread in in background and do not wait.
bool a = t1.joinable(); //Was thread already joined or detached.
			//If yes return true. Otherwise return false.

//Each thread need to be joined or detached otherwise there is runtime
//error.
----------------------------------------------------------------------------------
void f1(int & i);
int a1 = 5;
thread t1(f1, a1); //Providing input argument for function executed in
		   //separated thread by value.

void f2(int & i);
int a2 = 5; 
thread t2(f2, ref(a2)); //Providing input argument for function executed in
		        //separated thread by reference.

void f3(unique_ptr<int> a);
unique_ptr<int> a3 {new int{3}};
thread t3(f3, move(a3)); //moving input argument to function executed in
		         //separated thread.

----------------------------------------------------------------------------------
thread z(foo);
thread x = std::move(z); //Thread can be easilly moved but cant be copied.
x.join();
----------------------------------------------------------------------------------
vector<thread> threads;
//Create max number of thread supported by this device.
for(int i = 0; i < thread::hardware_concurrency(); i++)
{
    threads.push_back(thread([](int k){cout << "k: " << k << endl;}, i));
}
//Wait for each thread to finish at the end.
for_each(threads.begin(), threads.end(), mem_fn(&thread::join));
----------------------------------------------------------------------------------
this_thread::get_id(); //Get current thread id.

thread z(foo);
c.get_id(); //Get current thread id.
----------------------------------------------------------------------------------
list<int> my_list;
mutex my_mutex; //Mutex to protect my_list shared for two threads.

void add_to_list(int new_value)
{
    my_mutex.lock();
    my_list.push_back(new_value);
    my_mutex.unlock();
}

bool list_contains(int value_to_find)
{
    my_mutex.lock();
    bool f = find(my_list.begin(), my_list.end(), value_to_find) != my_list.end();
    my_mutex.unlock();
    return f;
}
-----------------------------------------------------------------------------------
list<int> my_list;
mutex my_mutex;

void add_to_list(int new_value)
{
    lock_guard<mutex> guard(my_mutex); //Its better to use RAII lock_guard instead 
    my_list.push_back(new_value);      //naked mutex. There is no need to remember
    cout << "Added to list: " << new_value << endl; //about unlock. 
}

bool list_contains(int value_to_find)
{
    lock_guard<mutex> guard(my_mutex);
    return find(my_list.begin(), my_list.end(), value_to_find) != my_list.end();
}
-----------------------------------------------------------------------------------
//Thread safe stack based on mutex and stack.
struct empty_stack : exception
{
};

template <typename T>
class threadsafe_stack
{
private:
    stack<T> data;
    mutable mutex m;
public:
    threadsafe_stack(){}
    threadsafe_stack(const threadsafe_stack & other)
    {
        lock_guard<mutex> lock(other.m);
        data = other.data;
    }

    threadsafe_stack & operator=(const threadsafe_stack & other ) = delete;

    void push(T new_value)
    {
        lock_guard<mutex> lock(m);
        data.push(new_value);
    }

    shared_ptr<T> pop()
    {
        lock_guard<mutex> lock(m);
        if(data.empty())
        {
            throw empty_stack();
        }
        shared_ptr<T> const res{make_shared<T>(data.top())};
        data.pop();
        return res;
    }
    void pop(T & value)
    {
        lock_guard<mutex> lock(m);
        if(data.empty())
        {
            throw empty_stack();
        }
        value = data.top();
        data.pop();
    }
    bool empty() const
    {
        lock_guard<mutex> lock(m);
        return data.empty();
    }
};
-----------------------------------------------------------------------------------
struct X
{
    list<T> & obj;
    mutex m;
};
friend void swap1(X & lhs, X & rhs)
{
    if(&lhs == &rhs) return;
    lock(lhs.m, rhs.m);//Lock 2 mutexes in the same time to protect dead lock.
    lock_guard<mutex> locka(lhs.m, adopt_lock);//Guard won't lock mutex thanks to 
    lock_guard<mutex> lockb(rhs.m, adopt_lock);//adopt_lock arg and will just take
    swap(lhs.obj, rhs.obj);		       //to unlock.
}

friend void swap2(X & lhs, X & rhs)
{
    if(&lhs == &rhs) return;
    unique_lock<mutex> locka(lhs.m, defer_lock);//unique_lock won't lock mutex thanks to 
    unique_lock<mutex> lockb(rhs.m, defer_lock);//defer_lock arg and will just take to unlock.
    lock(locka, lockb);//Lock 2 mutexes via unique_lock in the same time to protect dead lock.
    swap(lhs.obj, rhs.obj);
}
-----------------------------------------------------------------------------------
static thread_local unsigned int i;//Static instance common for one thread.
                                   //Separated instance for differen threads.
-----------------------------------------------------------------------------------
class CommSlot
{
    once_flag of; //Flag taking care that function provided to call_one function
                  //is really called once.

    void open(const string & s) {       //This function will be called one from
        cout << "open() " << s << endl; //send() or recive() function.
    }
public:
    void send() {
        call_once(of, &CommSlot::open, this, "from send()"); //Call open if wasn't 
        cout << "send()" << endl;                            //called earlier.
    }
    void receive() {
        call_once(of, &CommSlot::open, this, "from receive"); //Call open if wasn't
        cout << "receive()" << endl;                          //called earlier
    }
};
-----------------------------------------------------------------------------------
#include <boost/thread/shared_mutex.hpp>

class sharedVal
{
    int val{0};
    mutable  boost::shared_mutex entry_mutex; //Reader-writer mutex.
public:
    //Multiple function can share this mutex with shared_lock to read data.
    int read_val() const {
        boost::shared_lock<boost::shared_mutex> lk(entry_mutex);
        return val;
    }

    //Only one function can own this mutex with lock_guard/unique_lock to write data.
    void set_val(int & v) {
        std::lock_guard<boost::shared_mutex> lk(entry_mutex);
        val = v;
    }
};
-----------------------------------------------------------------------------------
#include <condition_variable>

mutex mut;
condition_variable data_cond; //Condition_variable from std.
int val;

void data_preparation_thread()
{
    for(int i = 1; i < 10; i++) {
        this_thread::sleep_for(1s);
        lock_guard<mutex> lk(mut);
        val = i;		  //Do expected work.
        data_cond.notify_one();   //When expected work done notify waiting thread.
    }
}

void data_process_thread()
{
    for(int i = 1; i < 10; i++) {
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [](){return !!val;}); //Wait for expected work to be done.
        cout << "data_proces_thread1 var: " << val << endl;
        val = 0;
   }
}
-----------------------------------------------------------------------------------
//Thread safe queue based on condition variable and std queue.

template<typename T>
class threadsafe_queue
{
private:
    mutex mut;
    queue<T> data_queue;
    condition_variable data_cond;

public:
    threadsafe_queue()
    {}

    threadsafe_queue(threadsafe_queue const & other)
    {
        lock_guard<mutex> lk(other.mut);
        data_queue = other.data_queue;
    }

    void push(T new_value)
    {
        std::lock_guard<mutex> lk(mut);
        data_queue.push(new_value);
        data_cond.notify_one();
    }

    void wait_and_pop(T& value)
    {
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [this](){return !data_queue.empty();});
        value = data_queue.front();
        data_queue.pop();
    }

    shared_ptr<T> wait_and_pop()
    {
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [this](){return !data_queue.empty();});
        shared_ptr<T> res = make_shared<T>(data_queue.front());
        data_queue.pop();
        return res;
    }

    bool try_pop(T& value)
    {
        lock_guard<mutex> lk(mut);
        if(data_queue.empty())
        {
            return false;
        }
        value = data_queue.front();
        data_queue.pop();
        return true;
    }

    shared_ptr<T> try_pop()
    {
        lock_guard<mutex> lk(mut);
        if(data_queue.empty())
        {
            return shared_ptr<T>(nullptr);
        }
        shared_ptr<T> res = make_shared<T>(data_queue.front());
        data_queue.pop();
        return res;
    }

    bool empty() const
    {
        std::lock_guard<mutex> lk(mut);
        return data_queue.empty();
    }
};
-----------------------------------------------------------------------------------
//Features future and async.

#include <future>

int returnVal()
{
    int i;
    for(i = 0; i < 4; i++)
    {
        cout << "returnVal i: " << i << endl;
        this_thread::sleep_for(2s);
    }
    return i;
}

void doSomeWork()
{
    for(int i = 0; i < 4; i++)
    {
        cout << "doSomeWork i: " << i << endl;
        this_thread::sleep_for(2s);
    }
}

int main()
{
    future<int> the_answer = async(returnVal);//Do function in background and return 
    doSomeWork();                             //future connected with function result.
    cout << "ret val is: " << the_answer.get() << endl;//Wait for background function
                                                       //and when completed return it
                                                       //result.
}


future<int> the_answer = async(returnVal); //Its up to implementation if function is
                                           //executed in background or current thread.
future<int> the_answer = async(launch::deferred | launch::async, returnVal);//As above.
future<int> the_answer = async(launch::deferred, returnVal);//Function executed in current
                                                            //thread when executed
                                                            //wait() or get() on future.
future<int> the_answer = async(launch::async, returnVal);//Function executed in background.

-----------------------------------------------------------------------------------

int f1() { //Some task to be done.
    cout << "f1" << endl;
    this_thread::sleep_for(1s);
    return 1;
}

void f2(packaged_task<int()> & task) {
    cout << "f2" << endl;
    future<int> fut = task.get_future(); //Get future of task.
    cout << fut.get() << endl;//Wait task to be compleded by another thread and use
}                             //this task result.

void f3(packaged_task<int()> & task) {
    cout << "f3" << endl;
    task();//Execute overwrapped task.
}

int main() {
    packaged_task<int()> task(f1); //Task is overwrapped by packaged_task.
    thread t1(f2, ref(task));
    thread t2(f3, ref(task));
    t1.join();
    t2.join();
}
-----------------------------------------------------------------------------------
class myException : public exception { //Notify waiting thread about unsuccessful
public:                                //work done by thread providing data.
    const char * what()  const throw() {
        return "my exception";
    }
};

void f1(promise<int> & pr, bool exceptionCase)
{
    if(!exceptionCase) {
        this_thread::sleep_for(1s); //Do some work successfully.
        pr.set_value(3);//Provide result to waiting thread/notify it about success.
    } else {
        try {
           this_thread::sleep_for(1s);//Do some work unsuccessfully.
           throw myException();//Work failed.
        } catch(...) { //Notify working thread about this fail.
           pr.set_exception(std::current_exception());
        }
    }
}

//void f2(future<int> & fut)//If only one waiting thread.
void f2(shared_future<int> & fut)//If more waiting thread.
{
    fut.wait();
    try {
        cout << fut.get() << endl; //Try to use data provided by another thread.
    } catch(myException & e) { //In case of error be prepared for exception.
        cout << e.what() << endl;
    }
}

void f3(shared_future<int> & fut)
{
    fut.wait();
    try {
        cout << fut.get() << endl; //Try to use data provided by another thread.
    } catch(myException & e) { //In case of error be prepared for exception.
        cout << e.what() << endl;
    }
}

int main()
{
    promise<int> pr1;
    future<int> fut1 = pr1.get_future();//If only one waiting thread.
    shared_future<int> sfut1(move(fut1));//If more waiting threads.
    thread t1(f1, ref(pr1), false);
    thread t2(f2, ref(sfut1));
    thread t3(f3, ref(sfut1));
    t1.join();
    t2.join();
    t3.join();

    promise<int> pr2;
    future<int> fut2 = pr2.get_future();
    shared_future<int> sfut2(move(fut2));
    thread t4(f1, ref(pr2), true);
    thread t5(f2, ref(sfut2));
    thread t6(f3, ref(sfut2));
    t4.join();
    t5.join();
    t6.join();

    return 0;
}
-----------------------------------------------------------------------------------
#include <chrono>

template<typename T>
void f1(promise<T> & pr)
{
    this_thread::sleep_for(5s);
    pr.set_value(5);
}

template<typename T>
void f2(shared_future<T> & fut)
{
    auto start = chrono::high_resolution_clock::now();
    cout << "f2: " << fut.get() << endl;
    auto stop = chrono::high_resolution_clock::now();
    //Measure time of waiting for future.
    cout << "f2 takes: " << chrono::duration_cast<chrono::milliseconds>(stop - start).count() << " ms" << endl;
}

template<typename T>
void f3(shared_future<T> & fut)
{
    //Wait for feature 3s and check if we have valid result or timeout.
    if(fut.wait_for(std::chrono::seconds(3)) == future_status::ready) {
        cout << "f3: " << fut.get() << endl;
    } else {
        cout << "f3: timeout" << endl;
    }
}

template<typename T>
void f4(shared_future<T> & fut)
{
    //Wait for feature 3s and check if we have valid result or timeout.
    if(fut.wait_for(std::chrono::seconds(6)) != future_status::timeout) {
        cout << "f4: " << fut.get() << endl;
    } else {
        cout << "f4: timeout" << endl;
    }
}

template<typename T>
void f5(shared_future<T> & fut)
{
    //Wait for feature till (curent time + 5s) and check if we have valid result or timeout.
    if(fut.wait_until(chrono::high_resolution_clock::now() + chrono::seconds(5)) == future_status::ready) {
        cout << "f5: " << fut.get() << endl;
    } else {
        cout << "f5: timeout" << endl;
    }
}

int main()
{
    promise<int> pr;
    shared_future<int> fut1(pr.get_future());
    shared_future<int> fut2(fut1);
    shared_future<int> fut3(fut1);
    shared_future<int> fut4(fut1);

    if(!fut1.valid()) return -1; //Is feature already paired with promise.

    thread t1(f1<int>, ref(pr));
    thread t2(f2<int>, ref(fut1));
    thread t3(f3<int>, ref(fut2));
    thread t4(f4<int>, ref(fut3));
    thread t5(f5<int>, ref(fut4));

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
}
-----------------------------------------------------------------------------------
//Sequential version of quick sort.
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
//Transforming sequential version of quick sort to parallel version of quick sort.
//There is only modification in 2 lines.
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
-----------------------------------------------------------------------------------
//For atomic type atomic_flag there are only 2 operations permitted.
//Write operation - clear()
//Read, modification, write operation - test_and_set()
#include <atomic>
using namespace std;

//Basic atomic type atomic_flag used to implement spin lock
class spinlock_mutex {
    atomic_flag flag; //Basic atomic type with only 2 operations: clear - write (to false).
                      //test_and_set - read current value. Modify (to true). Write.
public:
    spinlock_mutex() : flag(ATOMIC_FLAG_INIT) {}

    void lock() {
        while(flag.test_and_set(memory_order_acquire)) { //Read, modification, write operation.
            cout << "already locked, waiting" << endl;   //Write to true.
            this_thread::sleep_for(1s);
        }
        cout << "already unlocked, locking"<< endl;
    }

    void unlock() {
        flag.clear(memory_order_acquire);//Write operation. Write to false.
    }
};

void f1(spinlock_mutex & slm) {
    cout << "f1 begining" << endl;
    this_thread::sleep_for(2s);
    slm.lock();
    cout << "f1 slm locked" << endl;
    this_thread::sleep_for(5s);
    slm.unlock();
    cout << "f1 slm unlocked" << endl;
}

void f2(spinlock_mutex & slm) {
    cout << "f2 begining" << endl;
    this_thread::sleep_for(2s);
    slm.lock();
    cout << "f2 slm locked" << endl;
    this_thread::sleep_for(2s);
    slm.unlock();
    cout << "f2 slm unlocked" << endl;
}

int main() {
    spinlock_mutex slm;

    thread t1(f1, ref(slm));
    this_thread::sleep_for(1s);
    thread t2(f2, ref(slm));
    t1.join();
    t2.join();
}
-----------------------------------------------------------------------------------
//For atomic type atomic_flag there are only 5 operations permitted.
//Write operation - store()
//Read operation - load()
//Read, modification, write operation - exchange()
//Compare, exchange operation compare_exchange_week() and compare_exchange_strong() 
#include <atomic>

//Atomic type atomic<bool> used to implement spin lock
class spinlock_mutex {
    atomic<bool> flag;

public:
    spinlock_mutex() {
        flag.store(false);//Write operation.
        //Is this type in my implementation without internal blockads?
        cout << "is lock free: " << flag.is_lock_free() << endl;
    }

    void lock() {
        //Read, modification, write operation.
        while(flag.exchange(true, memory_order_acquire)) {
            cout << "already locked, waiting" << endl;
            this_thread::sleep_for(1s);
        }
        cout << "already unlocked, locking"<< endl;
    }

    void unlock() {
        flag.store(false);//Write operation.
    }

    bool try_lock() {
        //Read, modification, write operation.
        return !flag.exchange(true, memory_order_acquire);
    }
};

void f1(spinlock_mutex & slm) {
    cout << "f1 begining" << endl;
    this_thread::sleep_for(2s);
    slm.lock();
    cout << "f1 slm locked" << endl;
    this_thread::sleep_for(5s);
    slm.unlock();
    cout << "f1 slm unlocked" << endl;
}

void f2(spinlock_mutex & slm) {
    cout << "f2 begining" << endl;
    this_thread::sleep_for(2s);
    slm.lock();
    cout << "f2 slm locked" << endl;
    this_thread::sleep_for(5s);
    slm.unlock();
    cout << "f2 slm unlocked" << endl;
}

int main()
{
    spinlock_mutex slm;

    thread t1(f1, ref(slm));
    this_thread::sleep_for(1s);
    thread t2(f2, ref(slm));
    t1.join();
    t2.join();
}

-----------------------------------------------------------------------------------
//For atomic type atomic<bool> flag; there are following compare, exchange functions
//Function bool compare_exchange_week(bool & expected, bool new_value).
//Function compare current atomic type value with expected. And if these values are
//eqal set new_value. If they are not equal expected value is updated by function
//to current value. If operation was completed succesfully return true. Otherwise 
//return false. It could be that even if expected value was equal to current value
//new value was not set and functioncompare_exchange_week return false because of error.
//To be sure that writting was succesfull there is function compare_exchange_strong.

int main()
{
    atomic<bool> b;
    b.store(false);        //Set atomic flag to false so we expect
    bool expected = false; //there is false.

   cout << "begining, expected: " << expected << ", value: " << b.load() << endl;
   if(b.compare_exchange_weak(expected, true))//Change value to true.
   {   //Seting was succesfull so b = true and expected = false.
       cout << "1 set sucessfull, expected: " << expected << ", value: " << b.load() << endl;
   }
   else
   {   //Set was unsuccesfull, may happen for compare_exchange_weak
       //so b = false, expected = false
       cout << "1 set unsucessfull, expected: " << expected << ", value: " << b.load() << endl;
   }

   if(b.compare_exchange_strong(expected, true))
   {
       //If previous set was succesfull this should not happed.
       //Otherwise set was done sucesfully this time
       cout << "2 set sucessfull, expected: " << expected << ", value: " << b.load() << endl;
   }
   else
   {
       //If previous set was succesfull this time there was not set
       //so expected value was updated from false to true.
       //If previous set was unsuccesfull this won be called
       //because strong never fail.
       cout << "2 set unsucessfull, expected: " << expected << ", value: " << b.load() << endl;
   }
}
----------------------------------------------------------------------------------
//Type atomic<T*> have all the same function like atomic<bool> plus
//2 additional function fetch_add() and fetch_sub()
int a[5] = {1, 2, 3, 4, 5};
atomic<int*> p(a);
cout << *p.fetch_add(2) << endl;//1
cout << *p << endl;//3
cout << *p.fetch_sub(1) << endl;//3
cout << *p << endl;//2

----------------------------------------------------------------------------------
//Porzadkowanie spojne sekwencyjnie - memory_order_seq_cst. 
//If x is set before y in one thread. Second thread always see x was set before y 
//and never y was set before x. All thread see the same order of operations.

#include <iostream>
#include <atomic>
#include <thread>
using namespace std;

atomic<bool> x, y, z;

void write_x_then_y() {
    x.store(true, memory_order_seq_cst); //x is set before y
    y.store(true, memory_order_seq_cst);
}

void read_y_then_x()
{
    while(!y.load(memory_order_seq_cst));    //If we wait for y which is 
    if(x.load(memory_order_seq_cst)) {      //set after x we are sure
       z.store(true, memory_order_seq_cst); //x was also set
    }
}

int main() {
    x = y = z = false;

    thread a(write_x_then_y);
    thread b(read_y_then_x);
    a.join(); b.join();

    cout << "z must be 1, z = " << z.load(memory_order_seq_cst) << endl;
}

----------------------------------------------------------------------------------
//Porzadkowanie zlagodzone - memory_order_relaxed. 
//If x is set before y in one thread. Second thread can see x was set before y 
//or y was set before x. There is no order synchronization between threads.

#include <iostream>
#include <atomic>
#include <thread>
using namespace std;

atomic<bool> x, y, z;

void write_x_then_y() {
    x.store(true, memory_order_relaxed);//x is set before y.
    y.store(true, memory_order_relaxed);
}

void read_y_then_x()
{
    while(!y.load(memory_order_relaxed));//This thread can see
    if(x.load(memory_order_relaxed)) {   //y was set before x.
       z.store(true, memory_order_relaxed);
    }
}

int main() {
    x = y = z = false;

    thread a(write_x_then_y);
    thread b(read_y_then_x);
    a.join(); b.join();

    cout << "z can be 1 or 0, z = " << z.load(memory_order_relaxed) << endl;
}

----------------------------------------------------------------------------------
//Porzadkowanie przez wzajemne wykluczanie
//In this model reading operations are marked as memory_order_acquire,
//writting operations are marked as a memory_order_release and
//read-modification-write operations could be memory_order_acquire,
//memory_order_release or both memory_order_acq_rel.
//All threads are not synchronized but some pairs of threads writing and reading
//can be synchronized.

atomic<bool> x, y, z;

void write_x_then_y() {
    x.store(true, memory_order_relaxed);
    y.store(true, memory_order_release);//Writting y is synchronized with reading y.
}

void read_y_then_x() {
    while(!y.load(memory_order_acquire));//Reading y is synchronized with writting y.
    if(x.load(memory_order_relaxed)) {   //so reading x always return true.
        z.store(true, memory_order_release);
    }
}

int main() {
    x = y = z = false;

    thread a(write_x_then_y);
    thread b(read_y_then_x);
    a.join();
    b.join();
    cout << "z = always 1: " << z.load(memory_order_acquire) << endl;

    return 0;
}
