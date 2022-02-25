package lightfs;

import java.nio.ByteBuffer;

import static lightfs.LightUtils.*;

// LightFS file inode.
//
public class LightInodeFile extends LightInode
{
	protected
	LightInodeFile(LightBlockManager manager)
	{
		mType = BLOCK_TYPE_FILE;
		mManager = manager;
		mSize = 0;
		mBlockCount = 0;
		mChildren = new int[FILE_CAPACITY];
	}

	// Write the file into its block as a new entity, when it is just
	// created.
	protected void
	writeAsNew()
		throws Throwable
	{
		assert(mBlockId > 0);
		assert(mReadCount == 0);
		assert(!mIsWriting);
		// New file can't have any data.
		assert(mBlockCount == 0);
		mManager.checkSanity();
		ByteBuffer buf = mManager.takeBuffer();

		assert(buf.position() == mTypePos);
		buf.putInt(mType);

		assert(buf.position() == mSizePos);
		buf.putInt(mSize);

		assert(buf.position() == mNameLenPos);
		byte[] nameRaw = mName.getBytes();
		assert(nameRaw.length <= MAX_NAME_SIZE);
		assert(nameRaw.length > 0);
		buf.putInt(nameRaw.length);

		int pos = buf.position();
		assert(pos == mNamePos);
		buf.put(nameRaw);
		// Pad out to the block size. Even if the name is short. For
		// simplicity of reading the block from disk.
		buf.position(BLOCK_SIZE);
		buf.flip();
		mManager.writeBuffer(buf, mBlockId * BLOCK_SIZE);
		mIsLoaded = true;
	}

	// Load the file from its block.
	protected void
	load()
		throws Throwable
	{
		assert(mBlockId > 0);
		assert(mReadCount == 0);
		assert(!mIsWriting);
		mManager.checkSanity();
		if (mIsLoaded)
			return;

		ByteBuffer buf = mManager.takeBuffer();

		buf.position(BLOCK_SIZE);
		buf.flip();
		mManager.readBuffer(buf, mBlockId * BLOCK_SIZE);
		buf.flip();

		assert(buf.position() == mTypePos);
		mType = buf.getInt();
		assert(mType == BLOCK_TYPE_FILE);

		assert(buf.position() == mSizePos);
		mSize = buf.getInt();

		assert(buf.position() == mNameLenPos);
		int nameLen = buf.getInt();
		assert(nameLen <= MAX_NAME_SIZE);
		assert(nameLen > 0);

		int pos = buf.position();
		assert(pos == mNamePos);
		byte[] nameRaw = new byte[nameLen];
		buf.get(nameRaw);
		mName = new String(nameRaw);
		buf.position(pos + MAX_NAME_SIZE);

		assert(buf.position() == mChildredPos);
		for (int i = 0; i < FILE_CAPACITY; ++i) {
			int blockId = buf.getInt();
			if (blockId == 0)
				break;
			mChildren[i] = blockId;
			++mBlockCount;
		}
		mIsLoaded = true;
	}

	// Delete the file and all its content.
	protected void
	unlink()
		throws Throwable
	{
		assert(mBlockId > 0);
		assert(mIsLoaded);
		mManager.checkSanity();
		for (int i = 0; i < mBlockCount; ++i)
			delBlock();
		mManager.putBlock(mBlockId);
		mBlockId = 0;
		mIsLoaded = false;
	}

	// Write data into the file. Offset is the position in the file.
	protected void
	write(byte[] data, int offset)
		throws Throwable
	{
		mManager.checkSanity();
		int len = data.length;
		int newSize = offset + len;
		ensure(newSize);
		int blockIdx = offset / BLOCK_SIZE;
		int offsetInBlock = offset % BLOCK_SIZE;
		int dataOffset = 0;
		while (len > 0) {
			int dataPartLen = len;
			if (dataPartLen > BLOCK_SIZE)
				dataPartLen = BLOCK_SIZE;
			int blockId = mChildren[blockIdx];
			assert(blockId > 0);
			int fileOffset = blockId * BLOCK_SIZE + offsetInBlock;
			mManager.writeBytes(data, dataOffset, dataPartLen,
				fileOffset);
			len -= dataPartLen;
			dataOffset += dataPartLen;
			++blockIdx;
		}
		if (newSize > mSize)
			writeSize(newSize);
	}

	// Read data into the given buffer. Offset is the position in the file.
	// Returns how many bytes were actually read. Can be less that the
	// buffer capacity.
	protected int
	read(byte[] data, int offset)
		throws Throwable
	{
		if (offset >= mSize)
			return 0;
		mManager.checkSanity();
		int len = data.length;
		if (len + offset > mSize)
			len = mSize - offset;
		int result = len;
		int blockIdx = offset / BLOCK_SIZE;
		int dataOffset = 0;
		while (len > 0) {
			int dataPartLen = len;
			if (dataPartLen > BLOCK_SIZE)
				dataPartLen = BLOCK_SIZE;
			int blockId = mChildren[blockIdx];
			int fileOffset = blockId * BLOCK_SIZE;
			mManager.readBytes(data, dataOffset, dataPartLen,
				fileOffset);
			len -= dataPartLen;
			dataOffset += dataPartLen;
		}
		return result;
	}

	///////////////////////////// Private API //////////////////////////////

	// Make sure the file has enough blocks to fit the given size. Note, it
	// doesn't actually increase its size. Only its capacity.
	private void
	ensure(int capacity)
		throws Throwable
	{
		if (mSize >= capacity)
			return;
		int capacityHave = mBlockCount * BLOCK_SIZE;
		if (capacityHave >= capacity)
			return;

		int blockCountNeed = capacity / BLOCK_SIZE;
		if (capacity % BLOCK_SIZE != 0)
			++blockCountNeed;

		assert(blockCountNeed > mBlockCount);
		if (blockCountNeed > FILE_CAPACITY)
			throw new LightException.TooBig("file");

		int diff = blockCountNeed - mBlockCount;
		for (int i = 0; i < diff; ++i)
			addBlock();
	}

	// Append a new empty block to the file. Increases it capacity, not
	// size.
	private void
	addBlock()
		throws Throwable
	{
		assert(mBlockCount < FILE_CAPACITY);
		int blockId = mManager.takeBlock();
		int blockIdx = mBlockCount++;
		mChildren[blockIdx] = blockId;

		int offset = mChildredPos + blockIdx * UNIT_SIZE;
		int pos = mBlockId * BLOCK_SIZE + offset;
		ByteBuffer buf = mManager.takeBuffer();
		buf.putInt(blockId);
		buf.flip();
		mManager.writeBuffer(buf, pos);
	}

	// Pop the last block from the file. Doesn't change the size. It should
	// be changed beforehand.
	private void
	delBlock()
		throws Throwable
	{
		assert(mBlockCount > 0);
		int blockIdx = --mBlockCount;
		int blockId = mChildren[blockIdx];
		mChildren[blockIdx] = 0;
		mManager.putBlock(blockId);

		int offset = mChildredPos + blockIdx * UNIT_SIZE;
		int pos = mBlockId * BLOCK_SIZE + offset;
		ByteBuffer buf = mManager.takeBuffer();
		buf.putInt(blockId);
		buf.flip();
		mManager.writeBuffer(buf, pos);
	}

	// Persist the file size.
	private void
	writeSize(int size)
		throws Throwable
	{
		int pos = mBlockId * BLOCK_SIZE + mSizePos;
		ByteBuffer buf = mManager.takeBuffer();
		buf.putInt(size);
		buf.flip();
		mManager.writeBuffer(buf, pos);
		mSize = size;
	}

	protected static final int mSizePos = mTypePos + UNIT_SIZE;
	protected static final int mNameLenPos = mSizePos + UNIT_SIZE;
	protected static final int mNamePos = mNameLenPos + UNIT_SIZE;
	protected static final int mChildredPos = mNamePos + MAX_NAME_SIZE;
	protected LightBlockManager mManager;
	protected int mSize;
	protected int mBlockCount;
	protected int[] mChildren;
}
