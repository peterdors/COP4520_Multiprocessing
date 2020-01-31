import java.io.*;
import java.util.*;
import java.util.concurrent.*;
import static java.util.concurrent.TimeUnit.*;

public class DiningPhilosophers
{
	private ChopStick [] chopsticks; 
	private Philosopher [] philosophers;
	private Thread [] threads;

	public DiningPhilosophers(int numPhilosophers)
	{
		chopsticks = new ChopStick[numPhilosophers];
		philosophers = new Philosopher[numPhilosophers];
		threads = new Thread[numPhilosophers];

		for (int i = 0; i < numPhilosophers; i++)
		{
			chopsticks[i] = new ChopStick();
		}

		for (int i = 0; i < numPhilosophers; i++)
		{
			if (i % 2 == 0)
			{
				philosophers[i] = new Philosopher(i,  
										chopsticks[(i+1) % numPhilosophers], 
										chopsticks[i]);
			}
			else
			{
				philosophers[i] = new Philosopher(i, 
											chopsticks[i], 
											chopsticks[(i+1) % numPhilosophers]);
			}

			threads[i] = new Thread(philosophers[i]); 

			threads[i].start();
		}
	}

	public static void main(String [] args)
	{
		int numPhilosophers = 5; 
		DiningPhilosophers dp = 
			new DiningPhilosophers(numPhilosophers); 
	}
}
