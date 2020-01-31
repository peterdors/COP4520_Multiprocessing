import java.util.concurrent.*;
import java.io.*;
import java.time.*;

public class Philosopher implements Runnable
{
	private final int id; 
	private final ChopStick left, right; 

	public Philosopher(int id, ChopStick left, ChopStick right)
	{
		this.id = id; 
		this.left = left; 
		this.right = right;
	}

	@Override
	public void run() 
	{
		try
		{
			while (true)
			{
				System.out.println("Philosopher " + this.id + " is thinking. "
				 + LocalTime.now());
				Thread.sleep(1000);

				left.pickUp(); 
				right.pickUp(); 

				System.out.println("Philosopher " + this.id + " is eating with "
				 + "chopsticks " + left + " and " + right + ". " 
				 + LocalTime.now());

				Thread.sleep(1000);

				left.putDown(); 
				right.putDown();

				System.out.println("Philosopher " + this.id + " put down " +
					"their chopsticks. " + LocalTime.now());
			}

		}
		catch (InterruptedException ie)
		{
			Thread.currentThread().interrupt();
			return;
		}


	}
}
