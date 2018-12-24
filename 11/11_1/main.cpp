#include <iostream>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>
#include <thread>
#include <future>
#include <functional>
using namespace std;


//Framework start
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

//#define dispather_stub
//#define TemplateDispatcher_stub
class close_queue{}; //Message closing queue

#ifdef dispather_stub
    //Stub for dispatcher
    class dispatcher
    {
    public:
        dispatcher(queue *) {
            cout << "Stub dispatcher called" << endl;
        }
    };
#else //dispather_stub
#ifdef TemplateDispatcher_stub
    //Stub for TemplateDispatcher
    template<typename Dispatcher, typename Msg, typename Func>
    class TemplateDispatcher
    {
        TemplateDispatcher() {
            cout << "Stub TemplateDispatcher called" << endl;
        }
    };
#else //TemplateDispatcher_stub
    template<typename PreviousDispatcher, typename Msg, typename Func>
    class TemplateDispatcher
    {
        queue * q;
        bool chained;
        PreviousDispatcher * prev;
        Func f;

        //Different dispacher objects are friends to each other.
        template<typename Dispatcher, typename OtherMsg, typename OtherFunc>
        friend class TemplateDispatcher;

        //Can not copy dispatcher instances.
        TemplateDispatcher(TemplateDispatcher const &) = delete;
        TemplateDispatcher & operator=(TemplateDispatcher const &) = delete;


        //Wait for messages and route them.
        void wait_and_dispatch() {
            while(1) {
                auto msg = q->wait_and_pop();
                if(dispatch(msg)) break;
            }
        }

        //Check is message is type of close queue and if yes throw exception.
        bool dispatch(shared_ptr<message_base> const & msg) {
            cout << "dispatch called" << endl;
            //Cast to child class, check type of message and call function.
            if(wrapped_message<Msg> * wrapper = dynamic_cast<wrapped_message<Msg>*>(msg.get())) {
                f(wrapper->contents);
                return true;
            }
            return prev->dispatch(msg);//Send back to previous dispatcher.
        }
    public:
        TemplateDispatcher(TemplateDispatcher && other) :
            q(other.q), prev(other.prev), f(move(other.f)), chained(other.chained) {
            other.chained = true;
        }

        TemplateDispatcher(queue * q_, PreviousDispatcher * prev_, Func && f_) :
            q(q_), prev(prev_), f(std::forward<Func>(f_)), chained(false) {
               prev_->chained = true;
        }

        template<typename OtherMsg, typename OtherFunc>
        TemplateDispatcher<TemplateDispatcher, OtherMsg, OtherFunc>
        handle(OtherFunc && of) {
            return TemplateDispatcher<TemplateDispatcher, OtherMsg, OtherFunc>(q, this, std::forward<OtherFunc>(of));
        }

        ~TemplateDispatcher() noexcept(false) {
            if(!chained) {
                wait_and_dispatch();
            }
        }
    };
#endif //TemplateDispatcher_stub

    class dispatcher
    {
        queue * q;
        bool chained;

        //Can not copy dispatcher instances.
        dispatcher(dispatcher const &) = delete;
        dispatcher & operator=(dispatcher const &) = delete;

        //Instances of template TemplateDispatcher have access to
        //private dispatcher elements.
        template<typename Dispatcher, typename Msg, typename Func>
        friend class TemplateDispatcher;

        //Check is message is type of close queue and if yes throw exception.
        bool dispatch(shared_ptr<message_base> const & msg) {
            cout << "dispatch called" << endl;
            if(dynamic_cast<wrapped_message<close_queue>*>(msg.get())) {
                cout << "Sended close_queue" << endl;
                throw close_queue();
            }
            return false;
        }

        //Wait for messages and route them.
        void wait_and_dispatch() {
            while(1) {
                auto msg = q->wait_and_pop();
                dispatch(msg);
            }
        }

    public:
        //Instances of dispatcher can be moved.
        dispatcher(dispatcher && other) : q(other.q), chained(other.chained) {
            other.chained = true;   //Source cant wait for messages.
        }

        explicit dispatcher(queue * q_) : q(q_), chained(false) {
            cout << "Valid dispatcher called" << endl;
        }

        //Delegate function f and queue to new instance of TemplateDispatcher
        //which would be responsible for handling messege this type.
        template<typename Msg, typename Func>
        TemplateDispatcher<dispatcher, Msg, Func>
        handle(Func && f) {
            return TemplateDispatcher<dispatcher, Msg, Func>(q, this, std::forward<Func>(f));
        }

        ~dispatcher() noexcept(false) {
            if(!chained) {//If responsibility for handling messages was delegeted there is no
                wait_and_dispatch();//longer need for this dispatcher to take care of messages.
            }
        }
    };
#endif //dispather_stub

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
//Framework end

//Bankomat start
//Bankomat end

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
    try{
        messaging::receiver r3;
        messaging::sender s3{r3};
        s3.send(msgNaked);
        s3.send(messaging::close_queue());
        //Object dispatcher d3 in this line is not needed.
        messaging::dispatcher d3 = r3.wait();//1 solution
        //Or even more proper because dispatcher is temporary object.
        //auto fut = async(launch::async, &messaging::receiver::wait, &r3);//2 solution
        //fut.get();//2 solution
    } catch(messaging::close_queue & e) {
        cout << "Catched close_queue" << endl;
    }
    cout << "----- 3 ending -----" << endl;
}
