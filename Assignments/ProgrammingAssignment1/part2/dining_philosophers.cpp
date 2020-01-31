#include <array>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <random>
#include <iomanip>
#include <string_view>
#include <sstream>

using namespace std;

#define NUM_PHILOSOPHERS 5
mutex g_lockprint; 

struct chopstick
{
	mutex mtx; 
}; 


struct table 
{
	atomic<bool> ready{false};
	array<chopstick, NUM_PHILOSOPHERS> chopsticks;
}; 


struct philosopher
 {
 private: 
 	string const name; 
 	table &dinnertable;
 	chopstick &left, &right; 
 	// this thread. 
 	thread lifethread;
 	// Random number generator. 
 	mt19937 rng{ std::random_device{}() };
 public:
 	philosopher(string_view n, 
 		table &t, 
 		chopstick &l, 
 		chopstick &r):
 	
 		name(n), 
 		dinnertable(t),
 		left(l), 
 		right(r),
 		lifethread(&philosopher::dine, this)
	{
 	}

 	~philosopher()
 	{
 		lifethread.join();
 	}

 	void dine()
 	{
 		while (!dinnertable.ready)
 		{
 			;
 		}

 		while (dinnertable.ready)
 		{
 			think(); 
 			eat();
 		}
 	}

 	void print(string text)
 	{
 		lock_guard<mutex> cout_lock(g_lockprint);

 		cout << name << text << endl;
 	}

 	void think()
 	{
 		static thread_local uniform_int_distribution<> wait(1,6);
 		this_thread::sleep_for(chrono::milliseconds(wait(rng) * 150));

 		print(" is thinking ");
 	}

 	void eat()
 	{
 		lock(left.mtx, right.mtx);

 		lock_guard<mutex> leftLock(left.mtx, adopt_lock);
 		lock_guard<mutex> rightLock(right.mtx, adopt_lock);

 		print(" started eating ");

 		static thread_local uniform_int_distribution<> wait(1,6);
 		this_thread::sleep_for(chrono::milliseconds(wait(rng) * 150));

 		print(" finished eating ");
 	}
 	
}; 

void dine()
{
	this_thread::sleep_for(chrono::seconds(1));

	cout << "Dinner started." << endl;

	table table; 

	array<philosopher, NUM_PHILOSOPHERS> philosophers
	{
		{
			{"Aristotle", table, table.chopsticks[0], table.chopsticks[1]},
			{"Platon",    table, table.chopsticks[1], table.chopsticks[2]},
			{"Descartes", table, table.chopsticks[2], table.chopsticks[3]},
			{"Kant",      table, table.chopsticks[3], table.chopsticks[4]},
			{"Nietzsche", table, table.chopsticks[4], table.chopsticks[0]},
		}
	};

	table.ready = true;

	this_thread::sleep_for(chrono::seconds(5));
	
	table.ready = false;

	cout << "Dinner done." << endl;
}

int main(void)
{

	dine();


	return 0; 
}
