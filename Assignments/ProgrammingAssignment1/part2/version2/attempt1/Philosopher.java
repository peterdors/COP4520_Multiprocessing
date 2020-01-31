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
		// philosopher is thinking for some amount of time. 
		// Once a chopstick is available, the philosopher will feast for some 
		// amount of time and then put down the chop sticks once they have 
		// finished. 
		// The philosopher cannot be thinking and feasting at the same time. 
		try
		{
			while (true)
			{
				action(threadID + " is now thinking.");

				synchronized (leftChopStick)
				{
					action(threadID + " is now hungry.");

					synchronized (rightChopStick)
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
