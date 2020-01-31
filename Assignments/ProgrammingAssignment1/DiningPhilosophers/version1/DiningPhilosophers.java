// This implementation is liable to cause a dead-lock due to the 
// circularity of assigning chopsticks. Each philosopher will pickup the left 
// chopstick and each will end up waiting for the right chopstick to be 
// available.

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

			philosophers[i] = new Philosopher(leftChopStick, rightChopStick, 
												Integer.toString(i + 1));

			Thread t = new Thread(philosophers[i]);
			t.start();
		}
	}

}
