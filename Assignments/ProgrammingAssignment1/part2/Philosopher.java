public class Philosopher implements Runnable
{
	public Object leftChopStick, rightChopStick;
	public String threadID;

	Philosopher(Object leftChopStick, Object rightChopStick, String threadID)
	{
		this.leftChopStick = leftChopStick;
		this.rightChopStick = rightChopStick;
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
		String pid = "Philosopher " + threadID;

		try
		{
			while (true)
			{
				action(pid + " is thinking.");

				synchronized (leftChopStick)
				{
					action(pid + " picked up left chopstick.");

					synchronized (rightChopStick)
					{

						action(pid + " picked up right chopstick.");

						action(pid + " put down right chopstick.");
					}

					action(pid + " put down left chopstick. Back to thinking.");
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
		Thread.sleep((int) Math.random() * 100);
	}
}
