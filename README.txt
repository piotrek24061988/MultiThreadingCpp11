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

----------------------------------------------------------------------------------
//Fences - ogrodzenia.

atomic<bool> x, y, z;

void write_x_then_y() {
    x.store(true, memory_order_relaxed);//Code before fence
    atomic_thread_fence(memory_order_release);
    y.store(true, memory_order_relaxed);//Code after fence
}

void read_y_then_x() {
    while(!y.load(memory_order_relaxed));//Code before fence
    atomic_thread_fence(memory_order_acquire);
    if(x.load(memory_order_relaxed)) {//Code after fence
        z.store(true, memory_order_relaxed);
    }
}

int main() {
    x = y = z = false;

    thread a(write_x_then_y);
    thread b(read_y_then_x);

    a.join(); b.join();

    //Because of fence therew will be always 1.
    cout << "z = always 1: " << z.load() << endl;
}


//////////////////////////////////////////////////////
atomic<bool> x, y, z;

void write_x_then_y() {
    atomic_thread_fence(memory_order_release);
    x.store(true, memory_order_relaxed);
    y.store(true, memory_order_relaxed);
}

void read_y_then_x() {
    while(!y.load(memory_order_relaxed));
    atomic_thread_fence(memory_order_acquire);
    if(x.load(memory_order_relaxed)) {
        z.store(true, memory_order_relaxed);
    }
}

int main() {
    x = y = z = false;

    thread a(write_x_then_y);
    thread b(read_y_then_x);

    a.join(); b.join();

    //0 or 1 because writting x and y is not separated
    //with fence.
    cout << "z = 1 or 0: " << z.load() << endl;
}

///////////////////////////////

bool x, z;
atomic<bool> y;

void write_x_then_y() {
    x = true; //Code before fence
    atomic_thread_fence(memory_order_release);
    y.store(true, memory_order_relaxed); //Code after fence
}

void read_y_then_x() {
    while(!y.load(memory_order_relaxed)); //Code before fence
    atomic_thread_fence(memory_order_acquire);
    if(x) { //Code after fence
        z++;
    }
}

int main() {
    x = y = z = false;

    thread a(write_x_then_y);
    thread b(read_y_then_x);

    a.join(); b.join();

    //Always 1 because there are fences separating
    //x and y write and at least one type is atomic.
    cout << "z = always 1: " << z << endl;
}
----------------------------------------------------------------------------------
//Thread safe stack based on mutex and stack.
struct empty_stack : exception {};

template <typename T>
class threadsafe_stack {
private:
    stack<T> data;
    mutable mutex m;
public:
    threadsafe_stack(){}
    threadsafe_stack(const threadsafe_stack & other) {
        lock_guard<mutex> lock(other.m);
        data = other.data;
    }
    threadsafe_stack & operator=(const threadsafe_stack & other) = delete;

    void push(T new_value) {
        lock_guard<mutex> lock(m);
        data.push(move(new_value));
    }

    shared_ptr<T> pop() {
        lock_guard<mutex> lock(m);
        if(data.empty()) {
            throw empty_stack();
        }
        shared_ptr<T> const res(make_shared<T>(move(data.top())));
        data.pop();
        return res;
    }

    void pop(T & value) {
        lock_guard<mutex> lock(m);
        if(data.empty()) {
            throw empty_stack();
        }
        value = move(data.top());
        data.pop();
    }

    bool empty() const {
        lock_guard<mutex> lock(m);
        return data.empty();
    }
};

threadsafe_stack<int> tSs;
atomic<bool> endf;

void f1() {
    endf = false;
    for(auto & a : {1, 2, 3, 4, 5, 6, 7, 8 , 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19}) {
        tSs.push(a);
         this_thread::sleep_for(100ms);
    }
    this_thread::sleep_for(1s);
    endf = true;
}

void f2() {
    while(!endf) {
        if(!tSs.empty()) {
            try {
               cout << "f2: " << *(tSs.pop()) << endl;
            }
            catch(...) {
               cout << "f2: pop thrown exception" << endl;
            }
        }
    }
}

void f3() {
    while(!endf) {
        if(!tSs.empty()) {
            try {
                int val;
                tSs.pop(val);
                cout << "f3: " << val << endl;
            }
            catch(...) {
               cout << "f3: pop thrown exception" << endl;
            }
        }
    }
}

void f4() {
    while(!endf) {
        if(!tSs.empty()) {
            try {
                int val;
                tSs.pop(val);
                cout << "f4: " << val << endl;
            }
            catch(...) {
               cout << "f4: pop thrown exception" << endl;
            }
        }
    }
}

int main() {
   thread t2(f2);
   thread t3(f3);
   thread t4(f4);

   thread t1(f1);

   t1.join(); t2.join(); t3.join(); t4.join();
}

-----------------------------------------------------------------------------------
//Thread safe queue based on condition variable and std queue.

template<typename T>
class threadsafe_queue {
private:
    mutable mutex mut;
    queue<T> data_queue;
    condition_variable data_cond;
public:
    threadsafe_queue(){}

    void push(T new_value) {
        lock_guard<mutex> lk(mut);
        data_queue.push(move(new_value));
        data_cond.notify_one();
    }

    void wait_and_pop(T & value) {
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [this](){return !data_queue.empty();});
        value = move(data_queue.front());
        data_queue.pop();
    }

    shared_ptr<T> wait_and_pop() {
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [this](){return !data_queue.empty();});
        shared_ptr<T> res(make_shared<T>(move(data_queue.front())));
        data_queue.pop();
        return res;
    }

    bool try_pop(T & value) {
        lock_guard<mutex> lk(mut);
        if(data_queue.empty()) {
            return false;
        }
        value = move(data_queue.front());
        data_queue.pop();
        return true;
    }

    shared_ptr<T> try_pop() {
        lock_guard<mutex> lk(mut);
        if(data_queue.empty()) {
            return shared_ptr<T>();
        }
        shared_ptr<T> res = make_shared<T>(move(data_queue.front()));
        data_queue.pop();
        return res;
    }

    bool empty() const {
        lock_guard<mutex> lk(mut);
        return data_queue.empty();
    }
};

threadsafe_queue<int> tSq;
atomic<bool> endf;
atomic<bool> endf2;

void f0() {
    endf = false;
    for(auto & a : {-1, -2, -3, -4, -5, -6, -7, -8 ,9, -10, -11, -12, -13, -14, -15, -16, -17, -18, -19}) {
        tSq.push(a);
        this_thread::sleep_for(200ms);
    }
    endf = true;
}

void f1() {
    endf2 = false;
    for(auto & a : {1, 2, 3, 4, 5, 6, 7, 8 ,9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19}) {
        tSq.push(a);
        this_thread::sleep_for(300ms);
    }
    endf2 = true;
}

void f2() {
    while(!endf || !endf2) {
        cout << "f2: " << *(tSq.wait_and_pop()) << endl;
    }
}

void f3() {
    while(!endf || !endf2) {
        int val;
        tSq.wait_and_pop(val);
        cout << "f3: " << val << endl;
    }
}

void f4() {
    while(!endf || !endf2) {
        int val;
        if(tSq.try_pop(val)) {
            cout << "f4: " << val << endl;
        }
    }
}

void f5() {
    while(!endf || !endf2) {
        int val;
        if(auto a = tSq.try_pop()) {
            cout << "f5: " << *a << endl;
        }
    }
}

int main() {
   thread t2(f2); thread t3(f3); thread t4(f4); thread t5(f5);
   thread t0(f0); thread t1(f1);

   t2.detach(); t3.detach();
   t4.join(); t5.join();
   t0.join(); t1.join();
}
----------------------------------------------------------------------------------
//Thread safe queue based on std::queue.

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

threadsafe_queue<int> tSq;
atomic<bool> endf;
atomic<bool> endf2;

void f0() {
    endf = false;
    for(auto & a : {-1, -2, -3, -4, -5, -6, -7, -8 ,9, -10, -11, -12, -13, -14, -15, -16, -17, -18, -19}) {
        tSq.push(a);
        this_thread::sleep_for(200ms);
    }
    endf = true;
}

void f1() {
    endf2 = false;
    for(auto & a : {1, 2, 3, 4, 5, 6, 7, 8 ,9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19}) {
        tSq.push(a);
        this_thread::sleep_for(300ms);
    }
    endf2 = true;
}

void f2() {
    while(!endf || !endf2) {
        cout << "f2: " << *(tSq.wait_and_pop()) << endl;
    }
}

void f3() {
    while(!endf || !endf2) {
        int val;
        tSq.wait_and_pop(val);
        cout << "f3: " << val << endl;
    }
}

void f4() {
    while(!endf || !endf2) {
        int val;
        if(tSq.try_pop(val)) {
            cout << "f4: " << val << endl;
        }
    }
}

void f5() {
    while(!endf || !endf2) {
        int val;
        if(auto a = tSq.try_pop()) {
            cout << "f5: " << *a << endl;
        }
    }
}

int main() {
   thread t2(f2); thread t3(f3); thread t4(f4); thread t5(f5);
   thread t0(f0); thread t1(f1);

   t2.detach(); t3.detach();
   t4.join(); t5.join();
   t0.join(); t1.join();
}
----------------------------------------------------------------------------------
//Thread safe queue from scratch.

template<typename T>
class queue {
private:
    struct node {
        shared_ptr<T> data;
        unique_ptr<node> next;
    };

    mutex head_mutex;
    unique_ptr<node> head;

    mutex tail_mutex;
    node * tail;

    condition_variable data_cond;

    node * get_tail() {
        lock_guard<mutex> tail_lock(tail_mutex);
        return tail;
    }

    unique_ptr<node> pop_head() {
        unique_ptr<node> old_head = move(head);
        head = move(old_head->next);
        return old_head;
    }

    unique_lock<mutex> wait_for_data() {
        unique_lock<mutex> head_lock(head_mutex);
        data_cond.wait(head_lock, [&](){return head.get()!=get_tail();});
        return move(head_lock);
    }

    unique_ptr<node> wait_pop_head() {
        unique_lock<mutex> head_lock(wait_for_data());
        return pop_head();
    }

    unique_ptr<node> wait_pop_head(T & value) {
        unique_lock<mutex> head_lock(wait_for_data());
        value = move(*head->data);
        return pop_head();
    }

    unique_ptr<node> try_pop_head() {
        lock_guard<mutex> head_lock(head_mutex);
        if(head.get() == get_tail()) {
            return unique_ptr<node>();
        }
        return pop_head();
    }

    unique_ptr<node> try_pop_head(T & value) {
        lock_guard<mutex> head_lock(head_mutex);
        if(head.get() == get_tail()) {
            return unique_ptr<node>();
        }
        value = move(*head->data);
        return pop_head();
    }

public:
    queue() : head(new node), tail(head.get()){}
    queue(const queue & other) = delete;
    queue & operator=(const queue & other) = delete;

    shared_ptr<T> try_pop() {
        unique_ptr<node> old_head = try_pop_head();
        return old_head ? old_head->data : shared_ptr<T>();
    }

    bool try_pop(T & value) {
        unique_ptr<node> old_head = try_pop_head(value);
        return old_head;
    }

    shared_ptr<T> wait_and_pop() {
        unique_ptr<node> const old_head = wait_pop_head();
        return old_head->data;
    }

    void wait_and_pop(T & value) {
        unique_ptr<node> const old_head = wait_pop_head(value);
    }

    void push(T new_value) {
        shared_ptr<T> new_data = make_shared<T>(move(new_value));
        unique_ptr<node> p(new node);
        {
            lock_guard<mutex> tail_lock(tail_mutex);
            tail->data = new_data;
            node * const new_tail = p.get();
            tail->next = move(p);
            tail = new_tail;
        }
        data_cond.notify_one();
    }

    void empty() {
        lock_guard<mutex> head_lock(head_mutex);
        return (head.get() == get_tail());
    }
};

atomic<bool> a{false};
atomic<bool> b{false};
atomic<bool> c{false};

void f0(queue<int> & q) {
    for(auto & i : {-1, -2, -3, -4, -5, -6, -7, -8, -9, -10}) {
        q.push(i);
        this_thread::sleep_for(500ms);
    }
    this_thread::sleep_for(500ms);
    a = true;
}

void f1(queue<int> & q) {
    for(auto & i : {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}) {
        q.push(i);
        this_thread::sleep_for(500ms);
    }
    this_thread::sleep_for(500ms);
    b = true;
}

void f2(queue<int> & q) {
    for(auto & i : {11, 12, 13, 14, 15, 16, 17, 18, 19}) {
        q.push(i);
        this_thread::sleep_for(500ms);
    }
    this_thread::sleep_for(500ms);
    c = true;
}

void f3(queue<int> & q) {
    while(!a || !b || !c) {
        if(auto a = q.try_pop()) {
            cout << "f3: " << *a << endl;
        }
    }
}

void f4(queue<int> & q) {
    while(!a || !b || !c) {
        if(auto a = q.try_pop()) {
            cout << "f4: " << *a << endl;
        }
    }
}

void f5(queue<int> & q) {
    while(!a || !b || !c) {
            cout << "f5: " << *q.wait_and_pop() << endl;
    }
}

void f6(queue<int> & q) {
    while(!a || !b || !c) {
            cout << "f6: " << *q.wait_and_pop() << endl;
    }
}

int main()
{
    //thread safe queue used in one thread
    queue<int> q;
    if(auto a = q.try_pop()) {
         cout << *a << endl;
    } else {
         cout << "no val" << endl;
    }

    q.push(1);
    cout << *(q.try_pop()) << endl << endl;

    q.push(2);
    cout << *(q.try_pop()) << endl << endl;

    q.push(3);
    q.push(4);
    cout << *(q.try_pop()) << endl;
    cout << *(q.try_pop()) << endl << endl;

    q.push(5);
    q.push(6);
    q.push(7);
    if(auto a = q.try_pop()) {
        cout << *a << endl;
    } else {
        cout << "no val" << endl;
    }
    if(auto a = q.try_pop()) {
        cout << *a << endl;
    } else {
        cout << "no val" << endl;
    }
    if(auto a = q.try_pop()) {
        cout << *a << endl;
    } else {
        cout << "no val" << endl;
    }
    if(auto a = q.try_pop()) {
        cout << *a << endl;
    } else {
        cout << "no val" << endl;
    }

    cout << endl << endl << endl;

    //thread safe queue used in many threads
    thread t0(f0, ref(q));
    thread t1(f1, ref(q));
    thread t2(f2, ref(q));

    thread t3(f3, ref(q));
    thread t4(f4, ref(q));

    thread t5(f5, ref(q));
    thread t6(f6, ref(q));

    t0.join(); t1.join(); t2.join();

    t3.join(); t4.join();

    t5.detach(); t6.detach();
}

----------------------------------------------------------------------------------
//Thread safe list from scratch.

template <typename T>
class threadsafe_list {
    struct node {
        mutex m;
        shared_ptr<T> data;
        unique_ptr<node> next;

        node() : next(nullptr), data(nullptr) {}
        node(T const & value) : next(nullptr), data(make_shared<T>(value)) {}
    };

    node head;

    public:
        threadsafe_list() {}

        ~threadsafe_list() {
            //Remove each element of this list.
            remove_if([](node const &){return true;});
        }

        threadsafe_list(threadsafe_list const & other)=delete;
        threadsafe_list & operator=(threadsafe_list const & other)=delete;

        //Add new element to list.
        void push_front(T const & value) {
            unique_ptr<node> new_node(new node(value));
            lock_guard<mutex> lk(head.m);
            new_node->next = move(head.next);
            head.next=move(new_node);
        }

        //For each element from list execute Function.
        template<typename Function>
        void for_each(Function f) {
            node * current = &head;
            unique_lock<mutex> lk(head.m);
            while(node * const next = current->next.get()) {
                unique_lock<mutex> next_lk(next->m);
                lk.unlock();;
                f(*next->data);
                current = next;
                lk=move(next_lk);
            }
        }

        //Return first element passing Predicate.
        template<typename Predicate>
        shared_ptr<T> find_first_if(Predicate p) {
            node * current = &head;
            unique_lock<mutex> lk(head.m);
            while(node * const next = current->next.get()) {
                unique_lock<mutex> next_lk(next->m);
                lk.unlock();
                if(p(*next->data)) {
                    return next->data;
                }
                current = next;
                lk = move(next_lk);
            }
            return shared_ptr<T>();
        }

        //Iterate over the list and remove each element
        //passing Predicate.
        template<typename Predicate>
        void remove_if(Predicate p) {
            node * current = &head;
            unique_lock<mutex> lk(head.m);
            while(node * const next = current->next.get()) {
                unique_lock<mutex> next_lk(next->m);
                if(p(*next->data)) {
                    unique_ptr<node> old_next = move(current->next);
                    current->next = move(next->next);
                    next_lk.unlock();
                } else {
                    lk.unlock();
                    current = next;
                    lk=move(next_lk);
                }
            }
        }
};

//Function adding elements to list.
void f1(threadsafe_list<int> & tl) {
    for(auto & i : {1, 2, 3, 4, 5}) {
        cout << "f1: " << i << endl;
        tl.push_front(i);
        this_thread::sleep_for(300ms);
    }
    this_thread::sleep_for(300ms);
}

//Function adding elements to list.
void f2(threadsafe_list<int> & tl) {
    for(auto & i : {11, 12, 13, 14, 15}) {
        cout << "f2: " << i << endl;
        tl.push_front(i);
        this_thread::sleep_for(300ms);
    }
    this_thread::sleep_for(300ms);
}

//Function printing all elements currently stored in list.
void f3(threadsafe_list<int> & tl) {
    for(int i = 0; i < 10; i++) {
        tl.for_each([](int & j){cout << "f3:" << j << " ";});
        cout << endl;
        this_thread::sleep_for(300ms);
    }
}

//Function searching for value in list. Searching for
//0 in first iteration, 1 in second iteration etc...
void f4(threadsafe_list<int> & tl) {
    for(int i = 0; i < 10; i++) {
        auto a = tl.find_first_if([&i](int & j){return i == j;});
        if(a) {
            cout << "f4: " <<  *a << endl;
        } else {
            cout << "f4: value not found "  << endl;
        }
        this_thread::sleep_for(300ms);
    }
}

//Function removing value from list. Removing
//20 in first iteration, 19 i second iteration etc...
void f5(threadsafe_list<int> & tl) {
    for(int i = 20; i > 0; i--) {
        tl.remove_if([&i](int & j){return i == j;});
        this_thread::sleep_for(300ms);
    }
}

int main() {
    threadsafe_list<int> tl;

    thread t1(f1, ref(tl)); thread t2(f2, ref(tl));

    thread t3(f3, ref(tl)); thread t4(f4, ref(tl)); thread t5(f5, ref(tl));

    t1.join();  t2.join();

    t3.join(); t4.join(); t5.join();
}

----------------------------------------------------------------------------------
//Thread safe non blocking stack

template<typename T>
class lock_free_stack
{
private:
        atomic<unsigned> threads_in_pop;

        struct node {
            shared_ptr<T> data;
            node * next = nullptr;

            node(const T & data_) : data(make_shared<T>(data_)){}
        };

        atomic <node*> to_be_deleted;

        atomic<node*> head {nullptr};

        static void delete_nodes(node * nodes) {
            while(nodes) {
                node * next = nodes->next;
                delete nodes;
                nodes = next;
            }
        }

        void try_reclaim(node *old_head) {
            //Current node is used only by current pop (1 thread) so
            //can be deleted togeter with nodes waiting to be deleted.
            if(threads_in_pop == 1) {
                //Get list of nodes waiting to be deleted.
                node * nodes_to_delete = to_be_deleted.exchange(nullptr);
                //If there was no new pop instance in meantime called
                if(!--threads_in_pop) {
                    //Delete all nodes waiting for being deleted.
                    delete_nodes(nodes_to_delete);
                //If there was a new pop called in meantime deleting is
                //not safe so we need to push them back to be deleted later.
                } else if(nodes_to_delete) {
                    chain_pending_nodes(nodes_to_delete);
                }
                delete old_head;//delete current node directly.
            } else {//Current node is used by pop in other thread so can
                    //be only added to list of nodes to be deleted late.
                chain_pending_node(old_head);
                --threads_in_pop;
            }
        }

        void chain_pending_nodes(node * nodes) {
            node * last = nodes;
            //find end of list with node to be deleted
            while(node * const next = last->next) {
                last = next;
            }
            //Add to be deleted list to end of nodes list
            //and save first element of this chain as a
            //new node_to_be_deleted.
            chain_pending_nodes(nodes, last);
        }

        void chain_pending_nodes(node *first, node * last) {
            last->next = to_be_deleted;
            while(!to_be_deleted.compare_exchange_weak(last->next, first));
        }

        void chain_pending_node(node * n) {
            chain_pending_nodes(n, n);
        }

public:
        void push(const T & data) {
            node * new_node = new node(data); //Create new node
            new_node->next = head.load(); //Set new node next to current head.
            //If head is equal to new_node->next (what was requested in previous line
            //but could be modified by other thread), set head to new_node.
            while(!head.compare_exchange_weak(new_node->next, new_node));
        }

        //Version 3
        //issues: there is kind of internal garbage_collecter for elements
        //pop from stack because they can't be deleted directly. But if
        //there is have load of stack by using pop function all the time
        //garbage will never be removed and will grow till the end of
        //available memory.
        shared_ptr<T> pop() {
            ++threads_in_pop;
            node * old_head = head.load(); //Read value from current head.
            //If head is equeal to old_head(what was requested in precious line
            //but could be modified by other thread), set head to head->next,
            //to next node on the list.
            while(old_head && !head.compare_exchange_weak(old_head, old_head->next));

            shared_ptr<T> res;
            if(old_head) {
                res.swap(old_head->data);
            }
            try_reclaim(old_head);
            return res;
        }
};

void f1(lock_free_stack<int> & st) {
    for(auto & i : {1, 2, 3, 4, 5}) {
        st.push(i);
    }
}

void f2(lock_free_stack<int> & st) {
    for(auto & i : {6, 7, 8, 9, 10}) {
        st.push(i);
    }
}

void f3(lock_free_stack<int> & st) {
    for(int i = 0; i < 5; i++) {
        if(auto i = st.pop()) {
         cout << "f3: " << *i <<  " ";
        }
    }
}

void f4(lock_free_stack<int> & st) {
    for(int i = 0; i < 5; i++) {
        if(auto i = st.pop()) {
           cout << "f4: " << *i <<  " ";
        }
    }
}

int main()
{
    lock_free_stack<int> st;

    thread t1(f1, ref(st)); thread t2(f2, ref(st));

    thread t3(f3, ref(st)); thread t4(f4, ref(st));

    t1.join(); t2.join();

    t3.join(); t4.join();
    cout << endl;
}

----------------------------------------------------------------------------------
//Thread safe non blocking stack based on hazardPtr.

constexpr unsigned max_hazard_pointers = 100;

struct hazard_pointer {
    atomic<thread::id> id;//Id of thread using node pointed
    atomic<void*> pointer;//by pointer and cant be deleted.
};
hazard_pointer hazard_pointers[max_hazard_pointers];

class hp_owner
{
    hazard_pointer * hp;

public:
    hp_owner(const hp_owner &) = delete;
    hp_owner & operator=(const hp_owner &) = delete;

    hp_owner(): hp{nullptr} {
        //Find unused hazzardPtr from hazzardPtrList and asing it to current thread.
        for(unsigned i = 0; i < max_hazard_pointers; ++i) {
            thread::id old_id;
            if(hazard_pointers[i].id.compare_exchange_strong(old_id, this_thread::get_id())) {
                hp = &hazard_pointers[i];
                break;
            }
        }
        if(!hp) {
            throw runtime_error("No available hazard pointer in list");
        }
    }

    atomic<void*>& get_pointer() {
        return hp->pointer;
    }

    ~hp_owner() {
        hp->pointer.store(nullptr);
        //Default id indicating unsuded hazardPtr.
        hp->id.store(thread::id());
    }
};

//Each thread is owner of separated hazzardPtr.
atomic<void*>& get_hazard_pointer_for_current_thread() {
    thread_local static hp_owner hazard;
    return hazard.get_pointer();
}

//Check if any thread has hazzardPtr pointing to node
//provided as argument.
bool outstanding_hazard_pointers_for(void * p) {
    for(unsigned i = 0; i < max_hazard_pointers; ++i) {
        if(hazard_pointers[i].pointer.load() == p) {
            return true;
        }
    }
    return false;
}

template<typename T>
void do_delete(void * p) {
    delete static_cast<T*>(p);
}

struct data_to_reclaim {
    void * data;
    function<void(void*)> deleter;
    data_to_reclaim * next;

    template<typename T>
    data_to_reclaim(T * p): data(p), deleter(&do_delete<T>), next(0){}

    ~data_to_reclaim() {
        deleter(data);
    }
};

atomic<data_to_reclaim*> nodes_to_reclaim;

void add_to_reclaim_list(data_to_reclaim* node) {
    node->next = nodes_to_reclaim.load();
    while(!nodes_to_reclaim.compare_exchange_weak(node->next, node));
}

//Add node to list of nodes to be deleted later
template<typename T>
void reclaim_later(T *data) {
    add_to_reclaim_list(new data_to_reclaim(data));
}

//Delete all nodes added to list of nodes to be deleted later
//if they don't have owners (it there is no hazzardPointer for them)/
void delete_nodes_with_no_hazards() {
    data_to_reclaim * current = nodes_to_reclaim.exchange(nullptr);
    while(current) {
        data_to_reclaim * const next = current->next;
        if(!outstanding_hazard_pointers_for(current->data)) {
            delete current;
        } else {
            add_to_reclaim_list(current);
        }
        current = next;
    }
}

template<typename T>
class lock_free_stack
{
private:
        struct node {
            shared_ptr<T> data;
            node * next = nullptr;

            node(const T & data_) : data(make_shared<T>(data_)){}
        };

        atomic<node*> head {nullptr};
public:
        void push(const T & data) {
            node * new_node = new node(data); //Create new node
            new_node->next = head.load(); //Set new node next to current head.
            //If head is equal to new_node->next (what was requested in previous line
            //but could be modified by other thread), set head to new_node.
            while(!head.compare_exchange_weak(new_node->next, new_node));
        }
        //Version 4
        //Poping is quite slow because of complexity of used algorithm
        //to clean garbage.
        shared_ptr<T> pop() {
            atomic<void*> & hp = get_hazard_pointer_for_current_thread();
            node * old_head = head.load();
            do {
                node * temp;
                do {//HazardPtr should point to head.
                    temp = old_head;
                    //Old head cant be deleted by other thread because was
                    //added to HazardPtr.
                    hp.store(old_head);
                    old_head = head.load();
                //To double check if we really stored head in HazzardPtr and
                //head was not modified in meantime by other thread.
                } while(old_head != temp);
            } while(old_head && !head.compare_exchange_strong(old_head, old_head->next));
            //Clearing hazzard pointer which is no longer needed because we already get node.
            hp.store(nullptr);
            shared_ptr<T> res;
            if(old_head) {
                res.swap(old_head->data);
                //Before current node will be deleted we need to check if there are
                //other threads hazzard pointers pointing to this node.
                if(outstanding_hazard_pointers_for(old_head)) {
                    reclaim_later(old_head);//If yes mark to be deleted later.
                } else {
                    delete old_head;//If no delete directly.
                }
                //Checks all nodes marked for later delete with reclaim_later
                //function if there are ready to be deleted right now.
                delete_nodes_with_no_hazards();
            }
            return res;
        }
};

void f1(lock_free_stack<int> & st) {
    for(auto & i : {1, 2, 3, 4, 5}) {
        st.push(i);
    }
}

void f2(lock_free_stack<int> & st) {
    for(auto & i : {6, 7, 8, 9, 10}) {
        st.push(i);
    }
}

void f3(lock_free_stack<int> & st) {
    for(int i = 0; i < 5; i++) {
        if(auto i = st.pop()) {
         cout << "f3: " << *i <<  " ";
        }
    }
}

void f4(lock_free_stack<int> & st) {
    for(int i = 0; i < 5; i++) {
        if(auto i = st.pop()) {
           cout << "f4: " << *i <<  " ";
        }
    }
}

int main()
{
    lock_free_stack<int> st;

    thread t1(f1, ref(st)); thread t2(f2, ref(st));

    thread t3(f3, ref(st)); thread t4(f4, ref(st));

    t1.join(); t2.join();

    t3.join(); t4.join();
    cout << endl;

    return 0;
}

----------------------------------------------------------------------------------
//Parallel_acumulate suming values from container thread safe

class join_threads {
    vector<thread> & threads;
public:
    explicit join_threads(vector<thread> & threads_) : threads(threads_){}
    ~join_threads() {
        for(unsigned long i = 0; i < threads.size(); ++i) {
            if(threads[i].joinable()) threads[i].join();
        }
    }
};

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {

    unsigned long const length = distance(first, last);

    if(!length) {
        return init;
    }

    unsigned long const min_per_thread = 25;
    unsigned long const max_threads = (length+min_per_thread-1)/min_per_thread;
    unsigned long const hardware_threads = thread::hardware_concurrency();
    unsigned long const num_threads = min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
    unsigned long const block_size = length / num_threads;

    vector<future<T>> futures(num_threads - 1);
    vector<thread> threads(num_threads - 1);
    join_threads joiner(threads);
    auto accumulate_block = [](Iterator first, Iterator last) {
        cout << "thread_id: " << this_thread::get_id() << endl;
        return std::accumulate(first, last, T());
    };

    Iterator block_start = first;
    for(unsigned long i = 0; i < (num_threads - 1); ++i) {
            Iterator block_end = block_start;
            advance(block_end, block_size);
            std::packaged_task<T(Iterator, Iterator)> task(accumulate_block);
            futures[i] = task.get_future();
            threads[i] = thread(move(task), block_start, block_end);
            block_start = block_end;
    }
    T last_result = accumulate_block(block_start, last);
    T result = init;
    for(unsigned long i = 0; i < (num_threads - 1); ++i) {
        result += futures[i].get();
    }
    return (result += last_result);
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
    //Sequential sum of elements by std library
    sum = accumulate(li.begin(), li.end(), sum);
    cout << "time: " << chrono::duration<double>(chrono::system_clock::now() - start).count()  << endl;
    cout << "sequence accumaluate: " << sum  << endl;

    sum = 0;

    start = chrono::system_clock::now();
    //Parallel sum of elements by own parallel_function
    sum = parallel_accumulate(li.begin(), li.end(), sum);
    cout << "time: " << chrono::duration<double>(chrono::system_clock::now() - start).count() << endl;
    cout << "parallel accumaluate: " << sum  << endl;
}

----------------------------------------------------------------------------------
//Parallel_acumulate based on async suming values from container thread safe

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

----------------------------------------------------------------------------------
//Parallel for_each thread safe

class join_threads {
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

template<typename Iterator, typename Func>
void parallel_for_each(Iterator first, Iterator last, Func f) {
    unsigned long const length = distance(first, last);

    if(!length) return;

    unsigned long const min_per_thread = 25;
    unsigned long const max_threads = (length+min_per_thread-1)/min_per_thread;

    unsigned long const hardware_threads = thread::hardware_concurrency();

    unsigned long const num_threads = min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
    unsigned long const block_size = length / num_threads;

    vector<future<void>> futures(num_threads - 1);
    vector<thread> threads(num_threads - 1);
    join_threads joiner(threads);

    Iterator block_start = first;
    for(unsigned long i = 0; i < (num_threads - 1); ++i) {
            Iterator block_end = block_start;
            advance(block_end, block_size);
            std::packaged_task<void(void)> task([=]() {
                cout << "thread_id: " << this_thread::get_id() << endl;
                for_each(block_start, block_end, f);
            });
            futures[i] = task.get_future();
            threads[i] = thread(move(task));
            block_start = block_end;
    }

    for_each(block_start, last, f);
    for(unsigned long i = 0; i < (num_threads-1); ++i) {
        futures[i].get();
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

    cout << "before" << endl;
    for(auto &a : li) cout << a << " ";
    cout << endl;

    parallel_for_each(li.begin(), li.end(), [](int & i){i *= 2;});

    cout << "after" << endl;
    for(auto &a : li) cout << a << " ";
    cout << endl;
}

----------------------------------------------------------------------------------
//Parallel for_each thread safe based on async

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

----------------------------------------------------------------------------------
//Parallel thread safe find

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

template<typename Iterator, typename MatchType>
Iterator parallel_find(Iterator first, Iterator last, MatchType match)
{
    struct find_element
    {
        void operator()(Iterator begin, Iterator end, MatchType match,
                        promise<Iterator> * result, atomic<bool> * done_flag) {
            try {
                //Continue searching element by element or stop if another thread
                //already found value for his part of elements
                for(; (begin != end) && !done_flag->load(); ++begin) {
                    if(*begin==match) {//If this thread found value
                        result->set_value(begin);//save it place
                        done_flag->store(true);//and notify other threads
                        return;
                    }
                }
            } catch(...) {//If there is any exception we save it in promise
                try {     //and finish other threads
                    result->set_exception(current_exception());
                    done_flag->store(true);

                } catch(...) {}//Ignore any exception on this level
            }
        }
    };

    unsigned long const length = distance(first, last);
    if(!length) return last;

    unsigned long const min_per_thread = 25;
    unsigned long const max_threads = (length + min_per_thread-1)/min_per_thread;
    unsigned long const hardware_threads = thread::hardware_concurrency();
    unsigned long const num_threads = min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
    unsigned long const block_size = length/num_threads;

    promise<Iterator> result;
    atomic<bool> done_flag(false);
    vector<thread> threads(num_threads - 1);
    {
        join_threads joiner(threads);

        Iterator block_start = first;
        for(unsigned long i = 0; i < (num_threads-1); ++i) {
            Iterator block_end = block_start;
            advance(block_end, block_size);
            threads[i] = thread(find_element(), block_start, block_end, match, &result, &done_flag);
            block_start = block_end;
        }
        find_element()(block_start, last, match, &result, &done_flag);
    }
    if(!done_flag.load()) {//Searched element was not founded
        return last;
    }
    return result.get_future().get();//Provide searched element or exception
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
    cout << endl << endl;

    auto it = parallel_find(li.begin(), li.end(), 68);
    li.erase(it);

    cout << "after" << endl;
    for(auto &a : li) cout << a << " ";
    cout << endl << endl;
}

----------------------------------------------------------------------------------
//Parallel thread safe find based on async

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

----------------------------------------------------------------------------------
//Parallel thread safe partial_sum dividing container to sub containers.
Partial summing all values for every sub container. And adding last value
for every subcontainer from previous subcontainer

class join_threads //RAII to join created threads at the end.
{
    vector<thread> & threads;
public:
    explicit join_threads(vector<thread> & threads_) : threads(threads_) {}

    ~join_threads() {
        for(unsigned long i = 0; i < threads.size(); ++i) {
            if(threads[i].joinable()) {
                threads[i].join();
            }
        }
    }
};

//example of internal work
//[0, 1, 2, 3, 4, 5]
//thread 1 [0, x1+0, x2+0, x3+0, x4+0, x5+0]
//thread 2 [0, 1   , x2+1, x3+1, x4+1, x5+1]
//thread 3 [0, 1   , 3   , x3+2, x4+2, x5+2]
//thread 4 [0, 1   , 3   , 6   , x4+3, x5+3]
//thread 5 [0, 1   , 3   , 6   ,   10, x5+4]
//thread 6 [0, 1   , 3   , 6   ,   20, 15]
template <typename Iterator>
void parallel_partial_sum(Iterator first, Iterator last) {
    typedef typename Iterator::value_type value_type;

    struct process_element {
        void operator()(Iterator first, Iterator last, vector<value_type> & buffer,
                        unsigned i, barrier & b) {

            Iterator temp = first;
            advance(temp, i);
            value_type & ith_element = *temp;
            bool update_source = false;

            for(unsigned step = 0, stride = 1; stride <= i; ++step, stride *= 2) {
                temp = first;
                advance(temp, i - stride);
                value_type const & source = (step % 2) ? buffer[i] : ith_element;
                value_type & dest = (step % 2) ? ith_element : buffer[i];
                value_type const & addend = (step % 2) ? buffer[i - stride] : *temp;

                dest = source + addend;
                update_source = !(step % 2);

                b.wait();
            }
            //These 2 lines are just for debug
            cout << "thread_id: " << this_thread::get_id() << endl;
            for(auto i = first; i != last; i++) { cout << *i << " ";} cout << endl;
            if(update_source) {
                ith_element = buffer[i];
            }
            b.done_waiting();
        }
    };

    unsigned long const length = distance(first, last);

    if(length <= 1) return;

    vector<value_type> buffer(length);
    barrier b(length);

    vector<thread> threads(length - 1);//Number of threads depends on number of data
    join_threads joiner(threads);

    Iterator block_start = first;
    for(unsigned long i = 0; i < (length - 1); ++i) {
        threads[i] = thread(process_element(), first, last, ref(buffer), i, ref(b));
    }
    process_element()(first, last, buffer, length - 1, b);
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

    parallel_partial_sum(li.begin(), li.end());

    cout << "after" << endl;
    for(auto &a : li) cout << a << " ";
    cout << endl << endl;
}

----------------------------------------------------------------------------------
//Barrier defining number of thread which need to call blocking call wait
//to let this call be unloacked and let threads to continue they work.

class barrier
{
    atomic<unsigned> count;//Total number of threads
    atomic<unsigned> spaces;//Free places for threads
    atomic<unsigned> generation;

public:
    explicit barrier(unsigned count_) : count(count_), spaces(count_), generation(0){}

    void wait() {
        unsigned const my_generation = generation;
        if(!--spaces) {//Adding new waiting thread decrease number of free places.
            spaces = count.load();//If no more pleaces reasign initial value and
            ++generation;//let know waiting threads that they can continue they work.
        } else {
            while(generation==my_generation) {//If barier achieved, stop waiting.
                this_thread::yield();//Do not waiste processor time when waiting.
            }
        }
    }

    void done_waiting() {//Decrease total number of waiting threads to use
        --count;         //new lower value when current barier achieved.
        if(!--spaces) {//Decrease number of free places.
            spaces=count.load();//If no more pleaces reasign initial value and
            ++generation;//let know waiting threads that they can continue they work.
        }
    }
};

void f(barrier & b, string s)
{
    cout << s << " before" << endl;
    this_thread::sleep_for(1s);
    b.wait();
    cout << s <<" after" << endl;
}

int main()
{
    barrier b(3);

    thread t1(f, ref(b), "t1"); thread t2(f, ref(b), "t2"); thread t3(f, ref(b), "t3");

    t1.join(); t2.join(); t3.join();

    cout << endl << endl;

    thread t4(f, ref(b), "t4"); thread t5(f, ref(b), "t5");
    this_thread::sleep_for(1s);
    b.done_waiting();

    t4.join(); t5.join();
}

----------------------------------------------------------------------------------
//Partial_threadsafe_sum

class join_threads //RAII to join created threads at the end.
{
    vector<thread> & threads;
public:
    explicit join_threads(vector<thread> & threads_) : threads(threads_) {}

    ~join_threads() {
        for(unsigned long i = 0; i < threads.size(); ++i) {
            if(threads[i].joinable()) {
                threads[i].join();
            }
        }
    }
};

class barrier
{
    atomic<unsigned> count;//Total number of threads
    atomic<unsigned> spaces;//Free places for threads
    atomic<unsigned> generation;

public:
    explicit barrier(unsigned count_) : count(count_), spaces(count_), generation(0){}

    void wait() {
        unsigned const my_generation = generation;
        if(!--spaces) {//Adding new waiting thread decrease number of free places.
            spaces = count.load();//If no more pleaces reasign initial value and
            ++generation;//let know waiting threads that they can continue they work.
        } else {
            while(generation==my_generation) {//If barier achieved, stop waiting.
                this_thread::yield();//Do not waiste processor time when waiting.
            }
        }
    }

    void done_waiting() {//Decrease total number of waiting threads to use
        --count;         //new lower value when current barier achieved.
        if(!--spaces) {//Decrease number of free places.
            spaces=count.load();//If no more pleaces reasign initial value and
            ++generation;//let know waiting threads that they can continue they work.
        }
    }
};

//example of internal work
//[0, 1, 2, 3, 4, 5]
//thread 1 [0, x1+0, x2+0, x3+0, x4+0, x5+0]
//thread 2 [0,    1, x2+1, x3+1, x4+1, x5+1]
//thread 3 [0,    1,    3, x3+2, x4+2, x5+2]
//thread 4 [0,    1,    3,    6, x4+3, x5+3]
//thread 5 [0,    1,    3,    6,   10, x5+4]
//thread 6 [0,    1,    3,    6,   20, 15]
template <typename Iterator>
void parallel_partial_sum(Iterator first, Iterator last) {
    typedef typename Iterator::value_type value_type;

    struct process_element {
        void operator()(Iterator first, Iterator last, vector<value_type> & buffer,
                        unsigned i, barrier & b) {
            Iterator temp = first;
            advance(temp, i);
            value_type & ith_element = *temp;
            bool update_source = false;

            for(unsigned step = 0, stride = 1; stride <= i; ++step, stride *= 2) {
                temp = first;
                advance(temp, i - stride);
                value_type const & source = (step % 2) ? buffer[i] : ith_element;
                value_type & dest = (step % 2) ? ith_element : buffer[i];
                value_type const & addend = (step % 2) ? buffer[i - stride] : *temp;

                dest = source + addend;
                update_source = !(step % 2);
                b.wait();
            }
            if(update_source) {
                ith_element = buffer[i];
            }
            b.done_waiting();
        }
    };

    unsigned long const length = distance(first, last);

    if(length <= 1) return;

    vector<value_type> buffer(length);
    barrier b(length);

    vector<thread> threads(length - 1);//Number of threads depends on number of data
    join_threads joiner(threads);

    Iterator block_start = first;
    for(unsigned long i = 0; i < (length - 1); ++i) {
        threads[i] = thread(process_element(), first, last, ref(buffer), i, ref(b));
    }
    process_element()(first, last, buffer, length - 1, b);
}

int main()
{
    list<int> li = { 0, 1,  2,  3,  4,  5,  6,  7,  8,  9, 10,
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

    parallel_partial_sum(li.begin(), li.end());

    cout << "after" << endl;
    for(auto &a : li) cout << a << " ";
    cout << endl << endl;
}

----------------------------------------------------------------------------------
//First simple worker thread without notification when task added to worker is
completed. Otherwise function task can't return anything.

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
    threadsafe_queue<function<void()>> work_queue;
    vector<thread> threads;
    join_threads joiner;
    
    void worker_thread(unsigned i) {
        cout << "worker_thread: " << i << endl;
        while(!done) {
            function<void()> task;
            if(work_queue.try_pop(task)) {
                cout << "worker_thread: " << i << " doing task" << endl;
                task();
            } else {
                this_thread::yield();//tell scheduler to run other thread
            }
        }
    }

public:
    thread_pool() : done(false), joiner(threads) {
        unsigned const thread_count = thread::hardware_concurrency();
        try {
            for(unsigned i = 0; i < thread_count; i++) {
                threads.push_back(thread(&thread_pool::worker_thread, this, i));
            }
        } catch(...) {
            done = true;
            throw;
        }
    }

    ~thread_pool() {
        done = true;
    }

    template<typename FunctionType>
    void submit(FunctionType f) {
        work_queue.push(function<void()>(f));
    }
};

void f1() {
    cout << "f1" << endl;
    this_thread::sleep_for(1s);
}

void f2() {
    cout << "f2" << endl;
    this_thread::sleep_for(1s);
}

void f3() {
    cout << "f3" << endl;
    this_thread::sleep_for(1s);
}

int main()
{
    thread_pool tp;
    this_thread::sleep_for(5s);
    cout << "------------------------------" << endl;

    tp.submit(f1);
    this_thread::sleep_for(5s);
    cout << "------------------------------" << endl;

    tp.submit(f1);
    tp.submit(f2);
    this_thread::sleep_for(5s);
    cout << "------------------------------" << endl;

    tp.submit(f1);
    tp.submit(f2);
    tp.submit(f3);
    this_thread::sleep_for(5s);
    cout << "------------------------------" << endl;

    tp.submit(f1);
    tp.submit(f2);
    tp.submit(f3);
    tp.submit([](){cout << "f4" << endl; this_thread::sleep_for(1s);});
    this_thread::sleep_for(5s);
    cout << "------------------------------" << endl;
}

----------------------------------------------------------------------------------
//Second worker thread 

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

int f1() {
    cout << "f1" << endl;
    this_thread::sleep_for(1s);
    return 1;
}

int f2() {
    cout << "f2" << endl;
    this_thread::sleep_for(1s);
    return 2;
}

int f3() {
    cout << "f3" << endl;
    this_thread::sleep_for(1s);
    return 3;
}

int main()
{
    thread_pool tp;
    this_thread::sleep_for(1s);
    cout << "------------------------------" << endl;

    auto r1 = tp.submit(f1);
    cout << r1.get() << endl;
    cout << "------------------------------" << endl;

    auto r2 = tp.submit(f1);
    auto r3 = tp.submit(f2);
    cout << r2.get() << endl;
    cout << r3.get() << endl;
    cout << "------------------------------" << endl;

    auto r4 = tp.submit(f1);
    auto r5 = tp.submit(f2);
    auto r6 = tp.submit(f3);
    cout << r4.get() << endl;
    cout << r5.get() << endl;
    cout << r6.get() << endl;
    cout << "------------------------------" << endl;

    auto r7 = tp.submit(f1);
    auto r8 = tp.submit(f2);
    auto r9 = tp.submit(f3);
    auto r10 = tp.submit([](){cout << "f4" << endl; this_thread::sleep_for(1s); return 4;});
    cout << r7.get() << endl;
    cout << r8.get() << endl;
    cout << r9.get() << endl;
    cout << r10.get() << endl;
    cout << "------------------------------" << endl;
}

----------------------------------------------------------------------------------
//Parrallel_accumulate based on second worker_thread

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

----------------------------------------------------------------------------------

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
    vector<thread> threads;
    join_threads joiner;
    //Own function_wrapper for tasks to be done in thread_pool instead of
    //std::function<> to accept std::packaged_task<>.
    //Global queue common for threads need to be thread safe.
    threadsafe_queue<function_wrapper> pool_work_queue;
    typedef queue<function_wrapper> local_queue_type;
    //Local queue separated for each thread.If there is nothing in local queue.
    //Thread will take tasks from global queue. Local queue doesnt need to be
    //thread safe becaue is separated for eatch thread.
    static thread_local unique_ptr<local_queue_type> local_work_queue;

    void worker_thread() {
        cout << "thread_id: " << this_thread::get_id() << endl;
        //Create your own local thread.
        local_work_queue.reset(new local_queue_type);
        while(!done) {
            run_pending_task();
        }
    }

public:
    //Submit task to pool thread and get future associated with the result
    //of this task.
    template<typename FunctionType>
    future<typename result_of<FunctionType()>::type> submit(FunctionType f) {
        typedef typename result_of<FunctionType()>::type result_type;
        //Create packaged_task based on submited function. Wrap this function.
        packaged_task<result_type()> task(move(f));
        //Get future asociated with the result of this task.
        future<result_type> res(task.get_future());
        //If local queue exists push tasks there
        if(local_work_queue) {
            local_work_queue->push(move(task));
        } else {//If not push to global comon for all threads.
            pool_work_queue.push(move(task));
        }
        return res;
    }

    void run_pending_task() {
        function_wrapper task;
        //If local queue exists take task from it.
        if(local_work_queue && !local_work_queue->empty()) {
            task = move(local_work_queue->front());
            local_work_queue->pop();
            task();//If not try from global queue.
        }  else if(pool_work_queue.try_pop(task)) {
            task();
        } else {
            this_thread::yield();
        }
    }

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
};
thread_local unique_ptr<thread_pool::local_queue_type> thread_pool::local_work_queue;

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

----------------------------------------------------------------------------------

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

//Thread safe queue which is really wraper for
//std::deque with multithreading protection based on mutex
class work_stealing_queue {
private:
    typedef function_wrapper data_type;
    deque<data_type> the_queue;
    mutable mutex the_mutex;

public:
    work_stealing_queue(){}

    work_stealing_queue(const work_stealing_queue & other) = delete;
    work_stealing_queue & operator=(const work_stealing_queue & other) = delete;

    //Add task to the end.
    void push(data_type data) {
        lock_guard<mutex> lock(the_mutex);
        the_queue.push_front(move(data));
    }

    bool empty() const {
        lock_guard<mutex> lock(the_mutex);
        return the_queue.empty();
    }

    //Take taks from the end.
    bool try_pop(data_type & res) {
        lock_guard<mutex> lock(the_mutex);
        if(the_queue.empty()) {
            return false;
        }

        res = move(the_queue.front());
        the_queue.pop_front();
        return true;
    }

    //Steal task from the begining
    //oposite side to the end.
    bool try_steal(data_type & res) {
        lock_guard<mutex> lock(the_mutex);
        if(the_queue.empty()) {
            return false;
        }

        res = move(the_queue.back());
        the_queue.pop_back();
        return true;
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
    vector<thread> threads;
    join_threads joiner;
    //Own function_wrapper for tasks to be done in thread_pool instead of
    //std::function<> to accept std::packaged_task<>.
    //Global queue common for threads need to be thread safe.
    threadsafe_queue<function_wrapper> pool_work_queue;

    //Local queue separated for each thread.If there is nothing in local queue.
    //Thread will take tasks from global queue. Local queue doesnt need to be thread
    //safe becaue is separated for eatch thread. Nr of these queues is predefinied.
    vector<unique_ptr<work_stealing_queue>> queues;
    static thread_local work_stealing_queue * local_work_queue;
    static thread_local unsigned my_index;

    void worker_thread(unsigned my_index_) {
        cout << "thread_id: " << this_thread::get_id() << endl;
        my_index = my_index_;
        local_work_queue = queues[my_index].get();
        while(!done) {
            run_pending_task();
        }
    }

    //Take task from local queue separated for each thread with stealing mechanism.
    bool pop_task_from_local_queue(function_wrapper & task) {
        return local_work_queue && local_work_queue->try_pop(task);
    }

    //Take task from global thread safe queue common for all threads.
    bool pop_task_from_pool_queue(function_wrapper & task) {
        return pool_work_queue.try_pop(task);
    }

    //Try to steal task from other threads local queues.
    bool pop_task_from_other_thread_queue(function_wrapper & task) {
        for(unsigned i = 0; i < queues.size(); ++i) {
            unsigned const index = (my_index+i+1) % queues.size();
            if(queues[index]->try_steal(task)) {
                return true;
            }
        }
        return false;
    }

public:
    thread_pool() : done(false), joiner(threads) {
        unsigned const thread_count = thread::hardware_concurrency();

        try {
            for(unsigned i = 0; i < thread_count; i++) {
                //Create new local queue with stealing mechanism.
                queues.push_back(unique_ptr<work_stealing_queue>(new work_stealing_queue));
                //Create new thread.
                threads.push_back(thread(&thread_pool::worker_thread, this, i));
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
        //If local queue exists push tasks there
        if(local_work_queue) {
            local_work_queue->push(move(task));
        } else {//If not push to global comon for all threads.
            pool_work_queue.push(move(task));
        }
        return res;
    }

    void run_pending_task() {
        function_wrapper task;
        //Pop task from this local queue or from global queue of from other
        //thread local queue.
        if(pop_task_from_local_queue(task) || pop_task_from_pool_queue(task)
           || pop_task_from_other_thread_queue(task)) {
            task();
        } else {
            this_thread::yield();
        }
    }
};
thread_local work_stealing_queue * thread_pool::local_work_queue;
thread_local unsigned thread_pool::my_index;

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
    list<int> l { 198, 111, 34, 424, 56, 65, 2, 5, 3, 33, 46, 53, 64, 77, 87, 98, 556, 54, 78, 45, 87, 79, 88, 99, 5556,
                  564, 7, 8, 0, 678, 242, 465, 876, 43, 655, 5544, 766, 777, 800};
    l = parallel_quick_sort(l);

    for(auto & a : l)
    {
        cout << a << " ";
    }
    cout << endl;
}

----------------------------------------------------------------------------------
//Interruptible threads.

void interruption_point();

class interrupt_flag
{
    atomic<bool> flag;
    condition_variable * thread_cond;
    condition_variable_any * thread_cond_any;
    mutex set_clear_mutex;

public:
    interrupt_flag() : thread_cond(nullptr), thread_cond_any(nullptr) {}

    void set() {
        flag.store(true, memory_order_relaxed);
        lock_guard<mutex> lk(set_clear_mutex);
        if(thread_cond) {
            thread_cond->notify_all();
        } else if(thread_cond_any) {
            thread_cond_any->notify_all();
        }
    }

    template<typename Lockable>
    void wait(condition_variable_any & cv, Lockable & lk) {
        struct custom_lock
        {
            interrupt_flag * self;
            Lockable & lk;

            custom_lock(interrupt_flag * self_, condition_variable_any & cond, Lockable & lk_) : self(self_), lk(lk_) {
                self->set_clear_mutex.lock();
                self->thread_cond_any=&cond;
            }

            void unlock() {
                lk.unlock();
                self->set_clear_mutex.unlock();
            }

            void lock() {
                std::lock(self->set_clear_mutex, lk);
            }

            ~custom_lock() {
                self->thread_cond_any = 0;
                self->set_clear_mutex.unlock();
            }
        };

        custom_lock cl(this, cv, lk);
        interruption_point();
        cv.wait(cl);
        interruption_point();
    }

    bool is_set() const {
        return flag.load(memory_order_relaxed);
    }

    void set_condition_variable(condition_variable & cv) {
        lock_guard<mutex> lk(set_clear_mutex);
        thread_cond = &cv;
    }

    void clear_condition_variable() {
        lock_guard<mutex> lk(set_clear_mutex);
        thread_cond = 0;
    }
};
thread_local interrupt_flag this_thread_interrupt_flag;

struct clear_cv_on_destruct {
    ~clear_cv_on_destruct() {
        this_thread_interrupt_flag.clear_condition_variable();
    }
};

class thread_interrupted {};

//To be colled in function done in separated thread.
//Mark a point when this function execution can be interrupted.
//Thread let know when it can be interrupted. So if someone called
//interrupt on interruptible_thread, it will be interrupted in first
//interrupt_point.
void interruption_point() {
    if(this_thread_interrupt_flag.is_set()) {
        throw thread_interrupted();
    }
}

template <typename Predicate>
void interruptible_wait(condition_variable & cv, std::unique_lock<mutex> &lk, Predicate pred) {
    interruption_point();
    this_thread_interrupt_flag.set_condition_variable(cv);
    clear_cv_on_destruct guard;
    while(!this_thread_interrupt_flag.is_set() && !pred()) {
        cv.wait_for(lk, chrono::microseconds(1));
    }
    interruption_point();
}

template<typename Lockable>
void interruptible_wait(condition_variable_any & cv, Lockable &lk) {
    this_thread_interrupt_flag.wait(cv, lk);
}

template<typename T>
void interruptible_wait(future<T>& uf)
{
    while(!this_thread_interrupt_flag.is_set()) {
        if(uf.wait_for(chrono::microseconds(1)) == future_status::ready) {
            break;
        }
        interruption_point();
    }
}

//Thread which work can be interrupted based on std::thread.
struct interruptible_thread
{
    thread internal_thread;
    interrupt_flag * flag;

public:
    template <typename FunctionType>
    interruptible_thread(FunctionType f) {
        promise<interrupt_flag*>p;       //Initialize this_thread interupt_flag and run task.
        internal_thread=thread([f, &p](){
            p.set_value(&this_thread_interrupt_flag);//Set future to this_thread_interrupt_flag.
            try{
                f();
            }catch(thread_interrupted const &){
                cout << "thread_id: " << this_thread::get_id() << " interrupted" <<endl;
            }
        });//Asign interrupt_flag to thread_local this_thread_interrupt_flag taken from future.
        flag = p.get_future().get();//Ctor is finishing work when thread is running.
    }

    void join() {
        internal_thread.join();
    }

    void detach() {
        internal_thread.detach();
    }

    bool joinable() const {
        return internal_thread.joinable();
    }
    //Interrupt thread by calling set function on this_thread interrupt_flag.
    void interrupt() {
        if(flag) {
            flag->set();
        }
    }
};

void foo1() {
    for(int i = 0; i < 10; i++) {
        if(i >= 5) interruption_point();
        cout << "thread_id: " << this_thread::get_id() << ", i = " << i << endl;
        this_thread::sleep_for(1s);
    }
}

void foo2() {
    cout << "thread_id: " << this_thread::get_id() << " interruptible_wait1" << endl;
    mutex lk;
    condition_variable_any cv;
    interruptible_wait(cv, lk);
}

void foo3() {
    cout << "thread_id: " << this_thread::get_id() << " interruptible_wait2" << endl;
    promise<int> pr;
    future<int> fut = pr.get_future();
    interruptible_wait(fut);
}

void foo4() {
    cout << "thread_id: " << this_thread::get_id() << " interruptible_wait3" << endl;
    mutex m;
    unique_lock<mutex> lk(m);
    condition_variable cv;
    interruptible_wait(cv, lk, [](){return false;});
}

int main()
{
    //Thread finalized because of work done.
    vector<interruptible_thread> threads1;
    threads1.push_back(interruptible_thread(foo1));
    threads1.push_back(interruptible_thread(foo1));

    for(auto & t : threads1) {
        t.join();
    }

    //Threads finalized because of interruption.
    vector<interruptible_thread> threads2;
    threads2.push_back(interruptible_thread(foo1));
    threads2.push_back(interruptible_thread(foo1));

    this_thread::sleep_for(3s);

    for(auto & t: threads2) {
        t.interrupt();
    }
    for(auto & t : threads2) {
        t.join();
    }

    //Threads finalized because of interruption
    //during waiting.
    vector<interruptible_thread> threads3;
    threads3.push_back(interruptible_thread(foo2));
    threads3.push_back(interruptible_thread(foo2));

    this_thread::sleep_for(3s);

    for(auto & t: threads3) {
        t.interrupt();
    }
    for(auto & t : threads3) {
        t.join();
    }

    //Threads finalized because of interruption
    //during waiting2.
    vector<interruptible_thread> threads4;
    threads4.push_back(interruptible_thread(foo3));
    threads4.push_back(interruptible_thread(foo3));

    this_thread::sleep_for(3s);

    for(auto & t: threads4) {
        t.interrupt();
    }
    for(auto & t : threads4) {
        t.join();
    }

    //Threads finalized because of interruption
    //during waiting3.
    vector<interruptible_thread> threads5;
    threads5.push_back(interruptible_thread(foo4));
    threads5.push_back(interruptible_thread(foo4));

    this_thread::sleep_for(3s);

    for(auto & t: threads5) {
        t.interrupt();
    }
    for(auto & t : threads5) {
        t.join();
    }
}

----------------------------------------------------------------------------------
//Example tests for multithreading code.

//Thread safe queue based on condition variable and std queue.
template<typename T>
class threadsafe_queue {
private:
    mutable mutex mut;
    queue<T> data_queue;
    condition_variable data_cond;
public:
    threadsafe_queue(){}

    void push(T new_value) {
        lock_guard<mutex> lk(mut);
        data_queue.push(move(new_value));
        data_cond.notify_one();
    }

    void wait_and_pop(T & value, int ms) {
        unique_lock<mutex> lk(mut);
        if(data_cond.wait_for(lk, chrono::milliseconds(ms), [this](){return !data_queue.empty();})) {
            value = move(data_queue.front());
            data_queue.pop();
        }
    }

    shared_ptr<T> wait_and_pop(int ms) {
        shared_ptr<T> res;
        unique_lock<mutex> lk(mut);
        if(data_cond.wait_for(lk, chrono::milliseconds(ms), [this](){return !data_queue.empty();})) {
            res = make_shared<T>(move(data_queue.front()));
            data_queue.pop();
        } else {
            res = make_shared<T>();
        }
        return res;
    }

    bool try_pop(T & value) {
        lock_guard<mutex> lk(mut);
        if(data_queue.empty()) {
            return false;
        }
        value = move(data_queue.front());
        data_queue.pop();
        return true;
    }

    shared_ptr<T> try_pop() {
        lock_guard<mutex> lk(mut);
        if(data_queue.empty()) {
            return shared_ptr<T>();
        }
        shared_ptr<T> res = make_shared<T>(move(data_queue.front()));
        data_queue.pop();
        return res;
    }

    bool empty() const {
        lock_guard<mutex> lk(mut);
        return data_queue.empty();
    }
};

//Push data to empty queue in one thread.
//WaitPop data from this queue in second thread.
//In the end pop data should be equal to pushed data.
//In the end queue should be empty.
void test_concurrent_push_and_waitpop_on_empty_queue()
{
    threadsafe_queue<int> q; //Tested queue.

    constexpr int test_val = 42;

    promise<void> go, push_ready, pop_ready;
    shared_future<void> ready(go.get_future());

    future<void> push_done;
    future<shared_ptr<int>> pop_done;

    try {
        //Thread geting value from threadsafe_queue.
        pop_done = async(launch::async, [&q, ready, &pop_ready](){
            pop_ready.set_value();//Notify main thread that poping thread is started.
            ready.wait();//Wait for main thread to start test.
            return q.wait_and_pop(1000);
        });
        //Thread inserting value to threadsafe_queue.
        push_done = async(launch::async, [&q, ready, &push_ready](){
            push_ready.set_value();//Notify main thread that poping thread is started.
            ready.wait();//Wait for main thread to start test.
            q.push(test_val);
        });

        push_ready.get_future().wait();//Wait for poping thread to start.
        pop_ready.get_future().wait();//Wait for pushing thread to start.
        go.set_value();//Start test.

        //When push and waiting pop done returned value should be equal to inserted value
        //and queue should be empty at the end.
        push_done.get();
        assert(*(pop_done.get().get()) == test_val);
        assert(              q.empty() == true);
        cout << "test_concurrent_push_and_waitpop_on_empty_queue: passed" << endl;
    } catch(...) {
        go.set_value();
        throw;
    }
}

//Push data to empty queue in one thread.
//TryPop data from this queue in second thread.
//In the end pop data should be empty and queue should not be empty or.
//In the end pop data should be equal to pushed data and queue should be empty.
void test_concurrent_push_and_trypop_on_empty_queue()
{
    threadsafe_queue<int> q; //Tested queue.

    constexpr int test_val = 42;

    promise<void> go, push_ready, pop_ready;
    shared_future<void> ready(go.get_future());

    future<void> push_done;
    future<shared_ptr<int>> pop_done;

    try {
        //Thread geting value from threadsafe_queue.
        pop_done = async(launch::async, [&q, ready, &pop_ready](){
            pop_ready.set_value();//Notify main thread that poping thread is started.
            ready.wait();//Wait for main thread to start test.
            return q.try_pop();
        });
        //Thread inserting value to threadsafe_queue.
        push_done = async(launch::async, [&q, ready, &push_ready](){
            push_ready.set_value();//Notify main thread that poping thread is started.
            ready.wait();//Wait for main thread to start test.
            q.push(test_val);
        });

        push_ready.get_future().wait();//Wait for poping thread to start.
        pop_ready.get_future().wait();//Wait for pushing thread to start.
        go.set_value();//Start test.

        //When push and trypop done.
        //There should be no returned value and queue should not be empty.
        //Or returned value should be equal to inserted value
        //and queue should be empty at the end.
        push_done.get();
        auto poped = pop_done.get().get();
        auto empty = q.empty();
        assert( ( ( poped == nullptr)  && !q.empty() ) ||
                ( (*poped == test_val) &&  q.empty() ) );
        cout << "test_concurrent_push_and_trypop_on_empty_queue: passed" << endl;
    } catch(...) {
        go.set_value();
        throw;
    }
}

int main() {
    test_concurrent_push_and_waitpop_on_empty_queue();
    test_concurrent_push_and_trypop_on_empty_queue();
}
