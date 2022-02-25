package lightfs;

public class LightException
{
	public static class Corrupted extends Exception
	{
		public
		Corrupted(String message)
		{
			super(message);
		}
	}

	public static class InputOutput extends Exception
	{
		public
		InputOutput(String message)
		{
			super(message);
		}
	}

	public static class NotFound extends Exception
	{
		public
		NotFound(String message)
		{
			super(message);
		}
	}

	public static class InvalidArgs extends Exception
	{
		public
		InvalidArgs(String message)
		{
			super(message);
		}
	}

	public static class Busy extends Exception
	{
		public
		Busy(String message)
		{
			super(message);
		}
	}

	public static class Duplicate extends Exception
	{
		public
		Duplicate(String message)
		{
			super(message);
		}
	}

	public static class TooBig extends Exception
	{
		public
		TooBig(String message)
		{
			super(message);
		}
	}
}
