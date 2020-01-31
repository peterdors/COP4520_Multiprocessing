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

		BufferedReader in = 
			new BufferedReader(new InputStreamReader(System.in));
		String line = "";


		for (int i = 0; i < numPhilosophers; i++)
		{
			chopsticks[i] = new ChopStick();
		}

		for (int i = 0; i < numPhilosophers; i++)
		{
			philosophers[i] = new Philosopher(i, 
											chopsticks[i], 
											chopsticks[(i+1) % numPhilosophers]);

			threads[i] = new Thread(philosophers[i]); 

			threads[i].start();
		}

		for (Thread t : threads)
		{
			try 
			{
				t.join();
			}
			catch (InterruptedException ie)
			{
				Thread.currentThread().interrupt();
				return;
			}
		}
	}

	public static void main(String [] args)
	{
		int numPhilosophers = 5; 
		DiningPhilosophers dp = 
			new DiningPhilosophers(numPhilosophers); 
	}
}
