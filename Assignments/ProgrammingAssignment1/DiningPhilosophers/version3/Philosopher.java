public class Philosopher implements Runnable
{
	public Object firstChopStick, secondChopStick;
	public String threadID;

	// For version 3 we will now refer to the philosophers chopsticks as their
	// first and second.
	Philosopher(Object firstChopStick, Object secondChopStick, String threadID)
	{
		this.firstChopStick = firstChopStick;
		this.secondChopStick = secondChopStick;
		this.threadID = threadID;
	}

	@Override 
	public void run()
	{
		// While the philosopher is waiting for an available chopstick, the 
		// philosopher is thinking. 
		// Once a chopstick is available, the philosopher will feast for some 
		// amount of time and then put down the chop sticks once they have 
		// finished. 
		// The philosopher cannot be thinking and feasting at the same time. 
		try
		{
			while (true)
			{
				action(threadID + " is now thinking.");

				synchronized (firstChopStick)
				{
					action(threadID + " is now hungry.");

					synchronized (secondChopStick)
					{

						action(threadID + " is now eating.");
					}
				}
			}
		}
		catch (InterruptedException ie)
		{
			Thread.currentThread().interrupt();
			return;
		}
	}

	private void action(String message) throws InterruptedException
	{
		System.out.println(message);
		sleep();
	}

	private void sleep() throws InterruptedException
	{
		Thread.sleep((int) (Math.random() * 5000));
	}
}
