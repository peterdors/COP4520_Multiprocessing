// This version of the Dining Philosopher problem avoids starvation by using 
// assymetric acquirement; even philosophers starting with the right chopstick, 
// odd philosophers starting with the left chopstick. This solution is not 
// completely fair, but it does ensure that no philosopher will starve.

import java.util.*;
import java.io.*;

public class DiningPhilosophers
{
	public static void main(String [] args)
	{
		int numPhilosophers = 5;
		Philosopher [] philosophers = new Philosopher[numPhilosophers];
		Object[] chopsticks = new Object[numPhilosophers];
		Thread [] threads = new Thread[numPhilosophers];

		for (int i = 0; i < numPhilosophers; i++)
		{
			chopsticks[i] = new Object();
		}

		for (int i = 0; i < numPhilosophers; i++)
		{
			Object leftChopStick = chopsticks[i]; 
			Object rightChopStick = chopsticks[(i + 1) % numPhilosophers];

			if (i % 2 == 0)
			{
				philosophers[i] = new Philosopher(rightChopStick, leftChopStick, 
													Integer.toString(i + 1));
			}
			else 
			{
				philosophers[i] = new Philosopher(leftChopStick, rightChopStick, 
													Integer.toString(i + 1));
			}


			threads[i] = new Thread(philosophers[i]);
			threads[i].start();
		}
	}

}
