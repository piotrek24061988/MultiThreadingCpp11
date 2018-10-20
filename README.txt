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
