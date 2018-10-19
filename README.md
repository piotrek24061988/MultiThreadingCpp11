Multithreading tutorial in Cpp11

Theory:
----------------------------------------------------------------------
thread t1(foo);

t1.join(); // Wait for thread to finish.
t1.detach(); // Run thread in in background and do not wait.
bool a = t1.joinable(); //Was thread already joined or detached.
			//If yes return true. Otherwise return false.

//Each thread need to be joined or detached otherwise there is runtime
//error.
----------------------------------------------------------------------
