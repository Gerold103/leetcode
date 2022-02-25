package lightfs;

import static lightfs.LightUtils.*;

// File writer. Provides exclusive access to the file. New write and read
// streams can't be opened until this one is closed.
//
public class LightFileWriteStream
{
	protected
	LightFileWriteStream(LightInode inode, LightBlockManager manager)
		throws Throwable
	{
		if (inode == null)
			throw new LightException.NotFound("file");
		if (inode.mType != BLOCK_TYPE_FILE)
			throw new LightException.InvalidArgs("Not a file");
		if (inode.mIsWriting)
			throw new LightException.Busy("write");
		if (inode.mReadCount > 0)
			throw new LightException.Busy("read");

		inode.mIsWriting = true;
		mManager = manager;
		mInode = (LightInodeFile)inode;
		mPosition = 0;
	}

	// Write data into the file. Writes all the data or raises an exception.
	public void
	write(byte[] data)
		throws Throwable
	{
		mInode.write(data, mPosition);
		mPosition += data.length;
	}

	// Set writing position to the given value. Won't affect other streams.
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
		assert(mInode.mIsWriting);
		assert(mInode.mReadCount == 0);
		mInode.mIsWriting = false;
		mManager = null;
		mInode = null;
		mPosition = 0;
	}

	private LightBlockManager mManager;
	private LightInodeFile mInode;
	private int mPosition;
}
