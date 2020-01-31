import java.util.concurrent.*;


public class ChopStick
{
	private static class Stick 
	{

	}

	private Stick stick = new Stick();

	private BlockingQueue<Stick> holder = new ArrayBlockingQueue<>(1);

	public ChopStick()
	{
		putDown();
	}

	public void pickUp()
	{
		try
		{
			holder.take();
		}
		catch (InterruptedException ie)
		{
			Thread.currentThread().interrupt();
			return;
		}
	}

	public void putDown()
	{
		try
		{
			holder.put(stick);
		}
		catch (InterruptedException ie)
		{
			Thread.currentThread().interrupt();
			return;
		}
	}


}
