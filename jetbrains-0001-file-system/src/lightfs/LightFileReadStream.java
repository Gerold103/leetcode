package lightfs;

import static lightfs.LightUtils.*;

// File reader. Provides shared access for all other readers of the same file.
// Prevent writing to the same file until all readers are closed.
//
public class LightFileReadStream
{
	protected
	LightFileReadStream(LightInode inode, LightBlockManager manager)
		throws Throwable
	{
		if (inode == null)
			throw new LightException.NotFound("file");
		if (inode.mType != BLOCK_TYPE_FILE)
			throw new LightException.InvalidArgs("not a file");
		if (inode.mIsWriting)
			throw new LightException.Busy("write");

		++inode.mReadCount;
		mManager = manager;
		mInode = (LightInodeFile)inode;
		mPosition = 0;
	}

	// Read data into the given buffer. Returns how many bytes were actually
	// read. Can be less that the buffer capacity.
	public int
	read(byte[] data)
		throws Throwable
	{
		int rc = mInode.read(data, mPosition);
		mPosition += rc;
		return rc;
	}

	// Set reading position to the given value. Won't affect other streams.
	public void
	setPosition(int pos)
	{
		assert(pos >= 0);
		mPosition = pos;
	}

	// Free resources. The stream becomes unusable afterwards.
	public void
	close()
	{
		assert(mInode.mReadCount > 0);
		assert(!mInode.mIsWriting);
		--mInode.mReadCount;
		mManager = null;
		mInode = null;
		mPosition = 0;
	}

	private LightBlockManager mManager;
	private LightInodeFile mInode;
	private int mPosition;
}
