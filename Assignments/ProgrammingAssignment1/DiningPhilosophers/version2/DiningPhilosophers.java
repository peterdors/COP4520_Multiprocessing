// This implementation avoids deadlocking by breaking the circularity of 
// assigning chopsticks. The last philosopher will pickup the right chopstick 
// first and then the left chopstick. 

import java.util.*;

public class DiningPhilosophers
{
	public static void main(String [] args)
	{
		int numPhilosophers = 5;
		Philosopher [] philosophers = new Philosopher[numPhilosophers];
		Object[] chopsticks = new Object[numPhilosophers];

		for (int i = 0; i < numPhilosophers; i++)
		{
			chopsticks[i] = new Object();
		}

		for (int i = 0; i < numPhilosophers; i++)
		{
			Object leftChopStick = chopsticks[i]; 
			Object rightChopStick = chopsticks[(i + 1) % numPhilosophers];

			if (i + 1 == numPhilosophers)
			{
				philosophers[i] = new Philosopher(rightChopStick, leftChopStick, 
				                                    Integer.toString(i + 1));
			}
			else
			{
				philosophers[i] = new Philosopher(leftChopStick, rightChopStick, 
				                                    Integer.toString(i + 1));
			}

			Thread t = new Thread(philosophers[i]);
			t.start();
		}
	}

}
