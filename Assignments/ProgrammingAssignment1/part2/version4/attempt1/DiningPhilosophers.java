// This version of the Dining Philosopher problem uses the same solution as 
// version 3 where we assign the right chopstick first to the even numbered 
// philosopher and the left chopstick first to the odd numbered philosopher. 
// Except now we support a command line argument for the number of philosophers.

import java.util.*;
import java.io.*;

public class DiningPhilosophers
{
	public static void main(String [] args)
	{
		if (args.length < 1)
		{
			System.out.println("Usage: java DiningPhilosophers <num_philosophers>");
			System.exit(1); 
		}

		int numPhilosophers = Integer.parseInt(args[0]);
		
		Philosopher [] philosophers = new Philosopher[numPhilosophers];
		Thread [] threads = new Thread[numPhilosophers];

		Object[] chopsticks = new Object[numPhilosophers];

		for (int i = 0; i < numPhilosophers; i++)
		{
			chopsticks[i] = new Object();
		}

		for (int i = 0; i < numPhilosophers; i++)
		{
			Object leftChopStick = chopsticks[i % numPhilosophers]; 
			Object rightChopStick = chopsticks[(i + 1) % numPhilosophers];
			String threadID = Integer.toString(i + 1);

			if (i % 2 == 0)
			{
				philosophers[i] = new Philosopher(rightChopStick, leftChopStick, 
													threadID);
			}
			else 
			{
				philosophers[i] = new Philosopher(leftChopStick, rightChopStick, 
													threadID);
			}

			threads[i] = new Thread(philosophers[i]);
			threads[i].start();
		}
	}

}
