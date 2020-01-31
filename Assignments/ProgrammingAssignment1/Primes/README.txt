How to Run 
================================================================================
This program requires you to have the g++ compiler version (clang-1001.0.46.4)
installed. 

Use the included bash file to run from the command line by calling: 

bash run.sh

You can also the following calls from the command line:

g++ -std=c++11 -pthread primes.cpp && ./a.out


Summary of Approach
================================================================================
For this approach we use a version of the Sieve of Ertastothenes method for 
finding primes. In this implementation, we start 8 threads for each iteration
of a prime, and subsequently mark off every composite number that is a multiple 
of that prime number. 


Correctness and Efficiency of Algorithm
================================================================================
This algorithm uses a method that prevents the multiple threads that are running 
from "touching" a composite value more than once. This allows for us to avoid 
the need for mutual exclusion and using a shared counter. This overall allows 
for us to avoid the issue of thread contention. 

The efficiency of the algorithm also utilizes an efficient Sieve of 
Ertastothenes implementation which when done in a single thread implementation
also produces good runtime results. 

For example of the algorithm used, we start at the prime number 2. Then we run 
the 8 threads for marking off the multiples of 2, and we use a method where each 
thread is never touching the same number more than once for every prime number 
we encounter. 

So for the first thread were we are marking off the composite numbers for the 
prime 2, we will start a subsequent loop where our first thread starts at 4 and
will increment by (2 * <number of threads>) for each step. The second thread 
will start at 8 and then increment by (2 * <number of threads>) for each step. 
And so on for all 8 threads used. 

Once that first prime has its composite's marked off as not prime, we will go 
back to our function `findThePrimes(vector<thread *> &threads)` and start 
a new batch of 8 threads at the next subsequent prime value, 3. We will then do 
the same method of approach for marking off all the composite numbers for the 
prime value of 3. 

Summary of Experimental Evaluation
================================================================================
Testing this implementation by running the method 10 or fewer times has produced
an average runtime of about 1.7 seconds for completion. Also doing a sanity check 
against the sum of primes, count of primes, and last ten primes to check against
my results has proven through multiple runs of the program that we are getting 
the correct results every time. 


