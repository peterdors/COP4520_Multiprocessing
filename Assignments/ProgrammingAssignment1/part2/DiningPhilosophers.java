import java.util.*;

public class DiningPhilosophers
{
	static int numChopSticks = 5;

	public static void main(String [] args)
	{
		Philosopher [] philosophers = new Philosopher[numChopSticks];
		Object[] chopsticks = new Object[numChopSticks];

		for (int i = 0; i < numChopSticks; i++)
		{
			chopsticks[i] = new Object();
		}

		for (int i = 0; i < numChopSticks; i++)
		{
			Object leftChopStick = chopsticks[i]; 
			Object rightChopStick = chopsticks[(i + 1) % numChopSticks];

			// To have a deadlock (possibly) occur, remove the if-else
			// conditional below and uncomment the following lines.

			// philosophers[i] = new Philosopher(leftChopStick, rightChopStick, 
			// 									Integer.toString(i + 1));

			// This if-else prevents a deadlock from occurring when all the 
			// philosophers hold a chopstick and are waiting for the next one to
			// be available. 
			if (i == numChopSticks - 1)
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
