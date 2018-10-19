#include <iostream>
#include <map>
#include <mutex>
#include <boost/thread/shared_mutex.hpp>
#include <chrono>
#include <thread>
using namespace std;

class dns_entry
{
public:
    dns_entry(long unsigned int ip_ = 0)
        : ip(ip_)
    {
    }

    long unsigned int ip;
};

class dns_cache
{
    map<string, dns_entry> entries;
    mutable  boost::shared_mutex entry_mutex;
public:
    dns_entry find_entry(string const & domain) const
    {
        boost::shared_lock<boost::shared_mutex> lk(entry_mutex);
        map<string, dns_entry>::const_iterator const it = entries.find(domain);
        return (it == entries.end()) ? dns_entry() : it->second;
    }

    void update_or_add_entry(string const & domain, dns_entry const & dns_details)
    {
        std::lock_guard<boost::shared_mutex> lk(entry_mutex);
        entries[domain] = dns_details;
    }
};

dns_cache dC;

void f1()
{
    while(dC.find_entry("home").ip == 0)
    {
        cout << "entry not found" << endl;
        this_thread::sleep_for(1s);
    }
    cout << "entry found" << endl;
}

void f2()
{
    while(dC.find_entry("home").ip == 0)
    {
        cout << "entry not found2" << endl;
        this_thread::sleep_for(1s);
    }
    cout << "entry found2" << endl;
}

void f3()
{
    dC.update_or_add_entry("bla", dns_entry(1));
    this_thread::sleep_for(1s);
    dC.update_or_add_entry("bla", dns_entry(2));
    this_thread::sleep_for(1s);
    dC.update_or_add_entry("bla1", dns_entry(1));
    this_thread::sleep_for(1s);
    dC.update_or_add_entry("bla2", dns_entry(2));
    this_thread::sleep_for(1s);
    dC.update_or_add_entry("bla3", dns_entry(3));
    this_thread::sleep_for(1s);
    dC.update_or_add_entry("bla4", dns_entry(4));
    this_thread::sleep_for(1s);
    dC.update_or_add_entry("home", dns_entry(5));
}

int main()
{
    thread t1(f1);
    thread t2(f2);
    thread t3(f3);
    t1.join();
    t2.join();
    t3.join();
    return 0;
}

