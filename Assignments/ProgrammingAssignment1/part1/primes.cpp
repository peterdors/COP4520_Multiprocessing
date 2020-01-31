#include <cstring>  
#include <thread> 
#include <iostream> 
#include <fstream>
#include <vector> 
#include <chrono>

#define NUMTHREADS 8
#define MAXVAL ((int) 1e8)
#define NUMRUNS 10

using namespace std;
using namespace std::chrono;

// Creates a character array to represent the list of numbers from 0 to 
// (10^8 + 1). 
// Initializes every element in the array to be 1 for primes and 0 for 
// composite.
char *createAndInit(int n)
{
	char *temp = new char[n];
	temp[0] = 0;
	temp[1] = 0; 

	for (int i = 2; i < n; i++)
		temp[i] = 1;

	return temp;
}

void sieve(char *primes, int val, int thisThread)
{	
	uint64_t init = (val * val) + (val * thisThread);

	for (uint64_t i = init; i <= MAXVAL; i += (val * NUMTHREADS))
	{
		primes[i] = 0;
	}
}

// Function for finding and returning the primes, as well as getting the total 
// execution time to be averaged out back in the main function. 
char *findThePrimes(vector<thread *> &threads, duration<double> &executionTime)
{
	int val, thisThread; 
	clock_t start, end;

	// Represent the list of primes as characters as this provides us the best 
	// performance gains. 
	char *primes = createAndInit(MAXVAL + 1);
	
	for (val = 2; val * val <= MAXVAL; val++)
	{
		// Takes every composite number of that prime and uses the multithreaded
		// approach to mark them as not prime. Numbers are represented by their 
		// respective index and marked with a 1 for being prime and 0 for 
		// composite. 
		if (primes[val] == 1)
		{
			high_resolution_clock::time_point start = high_resolution_clock::now(); 

			for (thisThread = 0; thisThread < NUMTHREADS; thisThread++)
			{
				threads.push_back(new thread(sieve, primes, val, thisThread));
			}
		
			for (thisThread = 0; thisThread < NUMTHREADS; thisThread++)
			{
				threads[thisThread]->join();
			}

			high_resolution_clock::time_point end = high_resolution_clock::now();

			executionTime += duration_cast<duration<double>>(end - start);

			threads.clear();
		}
	}

	return primes; 
}

// Function for verifying our prime algorithm is getting the correct answers. 
void sanityCheck(int &cnt, uint64_t &sum, vector<int> &lastTen)
{
	// What the answers should be just for our sanity checks :).
	uint64_t primesSumAnswer = 279209790387276;
	int primesCntAnswer = 5761455;
	int lastTenPrimesAnswer [10] = {99999787, 99999821, 99999827, 
									99999839, 99999847, 99999931, 
									99999941, 99999959, 99999971, 
									99999989};
	bool passing = true; 

	for (int i = lastTen.size() - 1, k = 0; i >= 0 && k < 10; i--, k++)
	{
		if (lastTen[i] != lastTenPrimesAnswer[k])
		{
			passing = false;
		}
	}

	cout << "Sum check: "; 
	cout << (sum == primesSumAnswer ? "Hooray!" : "fail whale :(") << endl;
	cout << "Count check: ";
	cout << (cnt == primesCntAnswer ? "Hooray!" : "fail whale :(") << endl;
	cout << "Last ten primes check: ";
	cout << (passing ? "Hooray!" : "fail whale :(") << endl;
}

// Wrapper function for outputting the required results from our multithreaded 
// implementation. 
void printToFile(int &cnt, uint64_t &sum, 
	vector<int> &lastTen, duration<double> &averageTime)
{
	ofstream outputFile; 
	outputFile.open("primes.txt"); 

	outputFile << averageTime.count() << " seconds" << endl; 
	outputFile << cnt << endl;
	outputFile << sum << endl;

	for (int i = lastTen.size() - 1, k = 0; i >= 0 && k < 10; i--, k++)
	{
		outputFile << lastTen[i]; 
		outputFile << (i == 0 ? "\n" : " "); 
	}

	outputFile.close(); 
}

int main(void)
{
	int cnt = 0; 
	uint64_t sum = 0; 
	vector<int> lastTen;
	vector<thread *> threads;
	duration<double> executionTime, averageTime;

	char *primes = NULL;

	// Do a number of trial runs to get a good average of the time it takes 
	// for our multithreading to complete. 
	for (int i = 0; i < NUMRUNS; i++)
	{
		primes = findThePrimes(ref(threads), ref(executionTime));

		// Just for good memory management :).
		if (i + 1 < NUMRUNS)
		{
			delete primes;
		}
	}

	averageTime = executionTime / NUMRUNS;

	// Take the sum and count of all the primes. 
	for (int i = 2; i <= MAXVAL; i++)
	{
		if (primes[i] == 1)
		{
			cnt ++;
			sum += i;
		}
	}

	// Get the last ten primes from lowest to highest. 
	for (int i = MAXVAL, cnt = 0; i >= 0 && cnt < 10; i--)
	{
		if (primes[i])
		{
			cnt++;
			lastTen.push_back(i);
		}
	}

	// Just for making sure everything is a-okay.
	sanityCheck(ref(cnt), ref(sum), ref(lastTen));
	
	// Output the results to the file. 
 	printToFile(ref(cnt), ref(sum), ref(lastTen), ref(averageTime));

 	// Cleanup :).
 	delete primes;

	return 0;
}
