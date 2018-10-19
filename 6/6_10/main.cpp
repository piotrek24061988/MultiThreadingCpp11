#include <iostream>
#include <mutex>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <exception>
#include <stack>
#include <condition_variable>
#include <list>
#include <boost/thread/shared_mutex.hpp>
#include <algorithm>
using namespace std;

template <typename T>
class threadsafe_list
{
    struct node
    {
        mutex m;
        shared_ptr<T> data;
        unique_ptr<node> next;

        node() :
            next()
        {}
        node(T const & value)
            : data(make_shared<T>(value))
        {}
    };

    node head;

    public:
        threadsafe_list()
        {}

        ~threadsafe_list()
        {
            remove_if([](node const &){return true;});
        }

        threadsafe_list(threadsafe_list const & other)=delete;
        threadsafe_list & operator=(threadsafe_list const & other)=delete;

        void push_front(T const & value)
        {
            unique_ptr<node> new_node(new node(value));
            lock_guard<mutex> lk(head.m);
            new_node->next = move(head.next);
            head.next=move(new_node);
        }

        template<typename Function>
        void for_each(Function f)
        {
            node * current = &head;
            unique_lock<mutex> lk(head.m);
            while(node * const next = current->next.get())
            {
                unique_lock<mutex> next_lk(next->m);
                lk.unlock();;
                f(*next->data);
                current = next;
                lk=move(next_lk);
            }
        }

        template<typename Predicate>
        shared_ptr<T> find_first_if(Predicate p)
        {
            node * current = &head;
            unique_lock<mutex> lk(head.m);
            while(node * const next = current->next.get())
            {
                unique_lock<mutex> next_lk(next->m);
                lk.unlock();
                if(p(*next->data))
                {
                    return next->data;
                }
                current = next;
                lk = move(next_lk);
            }
            return shared_ptr<T>();
        }

        template<typename Predicate>
        void remove_if(Predicate p)
        {
            node * current = &head;
            unique_lock<mutex> lk(head.m);
            while(node * const next = current->next.get())
            {
                unique_lock<mutex> next_lk(next->m);
                if(p(*next->data))
                {
                    unique_ptr<node> old_next = move(current->next);
                    current->next = move(next->next);
                    next_lk.unlock();
                }
                else
                {
                    lk.unlock();
                    current = next;
                    lk=move(next_lk);
                }
            }
        }
};

void f1(threadsafe_list<int> & tl)
{
    for(auto & i : {1, 2, 3, 4, 5})
    {
        tl.push_front(i);
        this_thread::sleep_for(1s);
    }
    this_thread::sleep_for(1s);
}

void f2(threadsafe_list<int> & tl)
{
    for(auto & i : {11, 12, 13, 14, 15})
    {
        tl.push_front(i);
        this_thread::sleep_for(1s);
    }
    this_thread::sleep_for(1s);
}

void f3(threadsafe_list<int> & tl)
{
    for(int i = 0; i < 10; i++)
    {
        tl.for_each([](int & j){cout << "f3:" << j << " ";});
        cout << endl;
        this_thread::sleep_for(1s);
    }
}

void f4(threadsafe_list<int> & tl)
{
    for(int i = 0; i < 10; i++)
    {
        auto a = tl.find_first_if([&i](int & j){return i == j;});
        if(a)
        {
            cout << "f4: " <<  *a << endl;
        }
        else
        {
            cout << "f4: value not found "  << endl;
        }
        this_thread::sleep_for(1s);
    }
}

void f5(threadsafe_list<int> & tl)
{
    for(int i = 20; i > 0; i--)
    {
        tl.remove_if([&i](int & j){return i == j;});
        this_thread::sleep_for(1s);
    }
}

int main()
{
    threadsafe_list<int> tl;

    thread t1(f1, ref(tl));
    thread t2(f2, ref(tl));
    thread t3(f3, ref(tl));
    thread t4(f4, ref(tl));
    thread t5(f5, ref(tl));

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();

    return 0;
}

