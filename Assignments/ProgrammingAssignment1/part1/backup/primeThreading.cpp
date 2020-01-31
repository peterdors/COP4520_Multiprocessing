#include <iostream>
#include <mutex> 
#include <thread>
#include <cstring>
#include <atomic>
#include <vector>
#include <ctime>

using namespace std;

class Counter
{
  private:
    atomic<int> counter;
    atomic<unsigned long> sum;
    vector<unsigned long> list;
    mutex mtx;
    int maxVal;
    bool *primes;

  public:
    Counter(int maxVal)
    {
      this->maxVal = maxVal;
      this->counter = 2;
      primes = new bool[maxVal + 1];
      memset(primes, true, sizeof(bool) * maxVal + 1);
    }

    int getAndIncrement()
    {
      lock_guard<mutex> guard(mtx);
      
      counter++;

      return counter;
    }

    void addToSum(unsigned long val)
    {
      lock_guard<mutex> guard(mtx);

      sum += val;
    }

    unsigned long getSum()
    {
      return sum;
    }

    void printTopTen()
    {
      for (int i = 0; i < list.size(); i++)
      {
        if (i >= 10)
          break;

        cout << list[i] << " ";
      }

      cout << endl;
    }

    void appendTo(unsigned long val)
    {
      list.push_back(val);
    }

    void printPrimes()
    {
      for (int i = 2; i <= maxVal; i++)
      {
        if (primes[i])
        {
          cout << i << " ";
        }
      }

      cout << endl;
    }
};


void sieve(Counter *cntr, mutex *mtx, int maxVal, bool **primes)
{
  int i; 

  lock_guard<mutex> guard(mtx);

  // for (i = cntr->getAndIncrement(); i <= maxVal; i++)
  if ((i = cntr->getAndIncrement()) <= maxVal)
  {
    // i = counter->getAndIncrement();

    if ((*primes)[i])
    {
      cntr->addToSum((unsigned long) i);

      cntr->appendTo((unsigned long) i);

      for (int p = i * i; p <= maxVal; p += i)
      {
        (*primes)[p] = false;
      }
    }
  }
}

int main (void)
{
  int numThreads = 8;
  int maxVal = (int) 1e8;
  Counter *cntr = new Counter(maxVal);
  vector<thread *> threads; 

  // memset(primes, true, sizeof(bool) * maxVal + 1);

  clock_t start = clock();

  for (int i = 0; i < numThreads; i++)
  {
    threads.push_back(new thread(*cntr));
  }
  
  for (int i = 0; i < numThreads; i++)
  {
    threads[i]->join();
  }

  cout << "Done!\n";
  cout << "Time elapsed: " << (clock() - start) / CLOCKS_PER_SEC;
  cout << " seconds." << endl;
  cout << "Sum of all primes: " << cntr->getSum() << endl;
  cout << "Top ten: ";
  cntr->printTopTen();
  cout << endl;

  return 0;
}
