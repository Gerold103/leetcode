package lightfs;

import java.nio.ByteBuffer;

public class LightUtils
{
	protected static final int BLOCK_TYPE_DIR = 2;
	protected static final int BLOCK_TYPE_FILE = 3;

	protected static final int BLOCK_SIZE = 256;
	protected static final int BLOCK_FIRST_ID = 1;
	protected static final int MAX_NAME_SIZE = 64;
	protected static final int UNIT_SIZE = 4;
	protected static final int DIR_CAPACITY = 5;
	protected static final int FILE_CAPACITY = 5;

	protected static final int LIST_RECURSIVE = 1 << 0;
	protected static final int LIST_ONLY_FILES = 1 << 1;
	protected static final int LIST_ONLY_DIRS = 1 << 2;

	protected static String[]
	splitPath(String path)
	{
		String[] parts = path.split("/");
		int size = 0;
		for (String part : parts)
			size += part.isEmpty() ? 0 : 1;
		String[] res = new String[size];
		for (int i = 0, resi = 0; i < parts.length; ++i) {
			String part = parts[i];
			if (!part.isEmpty())
				res[resi++] = part;
		}
		return res;
	}
}
