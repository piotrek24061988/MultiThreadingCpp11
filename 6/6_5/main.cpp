#include <iostream>
#include <memory>
using namespace std;

//  tail (show las element of queue)                 head (show first element of queue)
//   |                                               |
//   V                                               V
// node <- node <- node <- node <- node <- node <- node
//
//
// empty list:
// tail    head
//     \  /
//     V  V
//     null
//
// one element list:
//     tail    head
//         \  /
//         V  V
// null <- node
//
//
// adding element to list:
//  tail     head
//   |(move)  |
//   V        V
// |node| <- node
//
//
// removing element from list:
//  tail     head
//   |        |(move)
//   V        V
//  node <- node <- |node|
template<typename T>
class queue {
private:
    struct node {
        T data;
        unique_ptr<node> next = nullptr;

        node(T data_) : data(move(data_)){}
    };

    unique_ptr<node> head = nullptr;//first element of queue
    node * tail = nullptr;//last element of queue
public:
    queue(){}
    queue(const queue & other) = delete;
    queue & operator=(const queue & other) = delete;

    void push(T new_value) {
        unique_ptr<node> p(make_unique<node>(move(new_value)));
        node * new_tail = p.get();
        if(tail) { //If queue wasn't empty move tail no new node.
            //tail->next = move(p);
            tail->next = move(p);
        } else {            //If queue was empty init head and
            head = move(p); //tail with the same new node.
        }
        tail = new_tail;
    }

    shared_ptr<T> try_pop() {
        if(!head) {
            return shared_ptr<T>();
        }
        shared_ptr<T> const res(make_shared<T>(move(head->data)));//Store node to be pop.
        unique_ptr<node> const old_head = move(head);
        head = move(old_head->next);//Move head to next node.
        if(tail->next == head) {//If next element was null
            tail = nullptr;     //we need to move tail to null also.
        }
        return res;
    }
};

int main() {
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
}
