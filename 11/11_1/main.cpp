#include <iostream>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>
using namespace std;

namespace messaging
{
    struct message_base //Base class for keeping all mesages in queue
    {
        virtual ~message_base(){}
    };

    template <typename Msg>
    struct wrapped_message : message_base //Child class specifying message.
    {
        Msg contents;

        explicit wrapped_message(Msg const & contents_) : contents(contents_){}
    };

    //Thread safe queue of messagess indicated by base class.
    class queue
    {
    private:
        mutex m;
        condition_variable c;
        std::queue<shared_ptr<message_base>> q;
    public:
        template<typename T>
        void push(T const & msg) {
            lock_guard<mutex> lk(m);
            q.push(make_shared<wrapped_message<T>>(msg));
            c.notify_all();
        }

        shared_ptr<message_base> wait_and_pop() {
            unique_lock<mutex> lk(m);
            c.wait(lk, [&](){return !q.empty();});
            auto res = q.front();
            q.pop();
            return res;
        }
    };

    class sender
    {
        queue * q; //Wraper for thread safe queue of messagess indicated by base class.

    public:
        sender() : q(nullptr) {}
        explicit sender(queue * q_) : q(q_){}

        template<typename Message>
        void send(Message const & msg) {
            if(q) {
                q->push(msg);
                cout << "Message added to queue sucesfully" << endl;
            } else {
                cout << "Queue not initialized" << endl;
            }
        }
    };

    //Stub for dispatcher
    class dispatcher
    {
    public:
        dispatcher(queue *) {
            cout << "Dispatcher stub called" << endl;
        }
    };

    class receiver {
        queue q; //Receiver is an owner of queue.
    public:
        operator sender() { //Casting reveiver to sender.
            return sender(&q);
        }

        dispatcher wait() {
            return dispatcher(&q);
        }
    };
}


int main()
{
    //--------------------------------1------------------------
    string msgNaked{"Hello"};
    messaging::queue q;
    q.push(msgNaked);
    cout << static_cast<messaging::wrapped_message<string> *>(q.wait_and_pop().get())->contents << endl;
    cout << "----- 1 ending -----" << endl;

    //--------------------------------2------------------------
    messaging::sender s1;
    s1.send(msgNaked);

    messaging::sender s2{&q};
    s2.send(msgNaked);
    cout << "----- 2 ending -----" << endl;

    //--------------------------------3------------------------
    messaging::receiver r3;
    messaging::sender s3{r3};
    messaging::dispatcher d3 = r3.wait();
    s3.send(msgNaked);
    cout << "----- 3 ending -----" << endl;

    return 0;
}

