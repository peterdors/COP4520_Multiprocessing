#include <cstring> 
#include <mutex> 
#include <thread> 
#include <iostream> 
#include <vector> 

using namespace std;

mutex mtx1, mtx2, mtx3, mtx4;
atomic<uint64_t> counter{2};
atomic<uint64_t> sum{0};

long getAndIncrement(void)
{
	return counter++;
}

uint64_t getNextPrimeAndIncrement(vector<bool> *primes)
{
	while (counter <= (uint64_t (1e8)) && !(*primes)[counter])
	{
		counter++;
	}

	return counter;
}

void incrementSum(uint64_t val)
{
	sum += val;
}

void sieve( vector<bool> *primes,
			uint64_t initVal,
			uint64_t maxVal)
{	
	uint64_t i = initVal; 

	for ( ; i <= maxVal ; )
	{	
		incrementSum(i);

		for (uint64_t p = i * i; p <= maxVal; p += i)
		{
			(*primes)[p] = false;
		}

		i = getNextPrimeAndIncrement(primes);
	}
}

// Single thread implementation for verifying correct output of sum in multi 
// threaded program.
uint64_t sumPrime(uint64_t maxVal)
{
	bool *primes = new bool[maxVal + 1];
	memset(primes, true, sizeof(bool) * (maxVal + 1));
	
	uint64_t sum = 0; 

	for (uint64_t i = 2; i * i <= maxVal; i++)
	{
		if (primes[i])
		{
			sum += (i);

			for (uint64_t p = i * i; p <= maxVal; p += i)
			{
				primes[p] = false;
			}
		}
	}

	return sum;
}

int main (void)
{
	uint64_t numThreads = 8;
	uint64_t maxVal = (uint64_t) 1e8; 
	vector<bool> primes(maxVal + 1, true);

	uint64_t initialPrimes [8] = {2,3,5,7,11,13,17,19};

	clock_t start = clock();
	uint64_t res = sumPrime(maxVal);
	clock_t end = clock(); 

	cout << "Single thread sum: " << (uint64_t) res << endl;
	cout << "Time elapsed: " << (end - start) / CLOCKS_PER_SEC;
	cout << " seconds." << "\n" << endl;

	vector<thread *> threads(numThreads);

	start = clock();

	for (uint64_t i = 0; i < numThreads; i++)
	{
		threads[i] = new thread(sieve, &primes, initialPrimes[i], maxVal);
	}
	
	for (uint64_t i = 0; i < numThreads; i++)
	{
		threads[i]->join();
	}

	end = clock();

	cout << "Multi Threading Complete!" << endl;
	cout << "Time elapsed: " << (end - start) / CLOCKS_PER_SEC;
	cout << " seconds." << endl;
	cout << "Sum: " << sum << endl;
	cout << "Counter: " << counter << endl;

	// Top ten max primes from highest to lowest.
	for (int i = primes.size() - 1, cnt = 0; i >= 0 && cnt < 10; i--)
	{
		if (primes[i])
		{
			cout << i << " ";
			cnt++;
		}
	}

	cout << endl;

	return 0;
}
