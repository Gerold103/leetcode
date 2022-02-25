package lightfs;

import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;

import static lightfs.LightUtils.*;

// The block manager allocates and frees blocks, does the actual reading and
// writing.
//
// LightFS consists of blocks of fixed size. Both metadata and data. The blocks
// are big enough for reasonable data amount and small enough for not too big
// overhead when store metadata.
//
// The blocks are identified by IDs which are numbers >= 1. Block location in a
// file can always be calculated as its ID * block size. Blocks are referenced
// by IDs when taken for new files or directories or for data.
//
// First block is a super one. It stores FS-wide meta:
//
// - Flag whether FS is 'dirty'. It is supposed to help with detection whether
//   the FS was left in an inconsistent state after last close. Although that
//   doesn't really work well in this implementation.
//
// - Total block count. It helps to understand how much of the FS' mother-file
//   is actually used. Because size of this file might be bigger than used by
//   LightFS due to any random reason.
//
// - Free block count. It helps to understand when it is time to allocate a new
//   block or re-use an existing free one.
//
// - Next free block ID.
//
// Free blocks are stored in a free-list. Super block stores first free block
// ID. That free block stores ID of a next free block. That free block stores ID
// of a next free block and so on. For example:
//
//     super:   nextFree: 10
//     block3:  nextFree: 9
//     block9:  nextFree: -
//     block10: nextFree: 15
//     block15: nextFree: 3
//
// That is a list of free blocks [10, 15, 3, 9]. When want to take block from
// the list, then the FS 'nextFree' from the first block. Returns the first
// block, and super starts pointing at that 'nextFree'. For example, this is how
// it looks after one pop:
//
//     super:   nextFree: 15
//     block3:  nextFree: 9
//     block9:  nextFree: -
//     block15: nextFree: 3
//
// The list now is [15, 3, 9]. When put a new free block, the new block sets its
// nextFree to the head block ID and becomes head itself. For example, here a
// block with ID 20 is added:
//
//     super:   nextFree: 20
//     block3:  nextFree: 9
//     block9:  nextFree: -
//     block15: nextFree: 3
//     block20: nextFree: 15
//
// The new list is [20, 15, 3, 9].
//
// On top of a proper block manager it becomes not so hard to implement the
// actual files, directories, and data blocks.
//
// The class is private to LightFS. It shouldn't be used outside of the package.
//
public class LightBlockManager
{
	// Create the manager on a new real file or open an existing one.
	protected
	LightBlockManager(String path)
		throws Throwable
	{
		mBuf = ByteBuffer.allocate(BLOCK_SIZE);
		mFile = new RandomAccessFile(path, "rws");
		mChan = mFile.getChannel();
		mIsDirty = false;
		mBlockCountTotal = 0;
		mBlockCountFree = 0;
		mNextFreeBlock = 0;
		if (mChan.size() != 0)
			openExisting();
		else
			openNew();
	}

	// Close the manager. The object becomes unusable after that.
	protected void
	close()
		throws Throwable
	{
		mChan = null;
		if (mFile != null)
			mFile.close();
		mFile = null;
		mIsDirty = true;
	}

	// How many blocks the FS has in total.
	protected int
	blockCount()
		throws Throwable
	{
		return mBlockCountTotal;
	}

	// Ensure the FS is in a consistent state. Otherwise an exception is
	// thrown.
	protected void
	checkSanity()
		throws Throwable
	{
		if (!mIsDirty)
			return;
		throw new LightException.Corrupted("The file system is broken");
	}

	// Take the global shared buffer. It is used for all read and write
	// operations in all streams to avoid unnecessary allocations.
	protected ByteBuffer
	takeBuffer()
		throws Throwable
	{
		mBuf.clear();
		return mBuf;
	}

	// Read the buf-limit bytes into the buffer. Starting from the given
	// position in the file. Reading less that the buf-limit leads to an
	// exception.
	protected void
	readBuffer(ByteBuffer buf, long pos)
		throws Throwable
	{
		int rem = buf.remaining();
		int rc = mChan.read(buf, pos);
		if (rc != rem) {
			throw new LightException.InputOutput(
				"Couldn't read a whole buffer");
		}
	}

	// Write the buf-position bytes into the buffer. Starting from the given
	// position in the file. Writing less that the buf-position leads to an
	// exception.
	protected void
	writeBuffer(ByteBuffer buf, long pos)
		throws Throwable
	{
		int rem = buf.remaining();
		int rc = mChan.write(buf, pos);
		if (rc != rem) {
			throw new LightException.InputOutput(
				"Couldn't write a whole buffer");
		}
	}

	// Same as writing a buffer, but write a byte array. Writes bytes
	// starting from data[dataOffset] until data[dataOffset + dataLen] into
	// the given position in the LightFS mother-file.
	protected void
	writeBytes(byte[] data, int dataOffset, int dataLen, long pos)
		throws Throwable
	{
		ByteBuffer buf = ByteBuffer.wrap(data, dataOffset, dataLen);
		writeBuffer(buf, pos);
	}

	// Almost the same as writing bytes.
	protected void
	readBytes(byte[] data, int dataOffset, int dataLen, long pos)
		throws Throwable
	{
		ByteBuffer buf = ByteBuffer.wrap(data, dataOffset, dataLen);
		readBuffer(buf, pos);
	}

	// Return the given block ID to the free blocks list.
	protected void
	putBlock(int blockId)
		throws Throwable
	{
		checkSanity();
		mIsDirty = true;
		ByteBuffer buf = takeBuffer();
		buf.put((byte)(mIsDirty ? 1 : 0));
		writeBuffer(buf, mIsDirtyPos);

		// Point at the old head.
		buf = takeBuffer();
		buf.putInt(mNextFreeBlock);
		int offset = blockId * BLOCK_SIZE;
		writeBuffer(buf, offset);

		// Set this block as a new head.
		buf = takeBuffer();
		buf.putInt(blockId);
		writeBuffer(buf, mNextFreeBlockPos);

		++mBlockCountFree;
		buf = takeBuffer();
		buf.putInt(mBlockCountFree);
		buf.flip();
		writeBuffer(buf, mBlockCountFreePos);

		buf = takeBuffer();
		buf.put((byte)0);
		writeBuffer(buf, mIsDirtyPos);
		mIsDirty = false;
	}

	// Take a new block for some usage. Might reuse an existing block from
	// the free list.
	protected int
	takeBlock()
		throws Throwable
	{
		checkSanity();
		reserveBlock();
		mIsDirty = true;
		ByteBuffer buf = takeBuffer();
		buf.put((byte)(mIsDirty ? 1 : 0));
		buf.flip();
		writeBuffer(buf, mIsDirtyPos);

		// Get next block after the head (if there is none - the head
		// becomes 0).
		int blockId = mNextFreeBlock;
		int offset = blockId * BLOCK_SIZE;
		buf = takeBuffer();
		buf.limit(UNIT_SIZE);
		readBuffer(buf, offset);
		buf.flip();
		mNextFreeBlock = buf.getInt();

		// Update the head. Now the old head is popped.
		buf = takeBuffer();
		buf.putInt(mNextFreeBlock);
		buf.flip();
		writeBuffer(buf, mNextFreeBlockPos);

		--mBlockCountFree;
		buf = takeBuffer();
		buf.putInt(mBlockCountFree);
		buf.flip();
		writeBuffer(buf, mBlockCountFreePos);

		buf = takeBuffer();
		buf.put((byte)0);
		buf.flip();
		writeBuffer(buf, mIsDirtyPos);
		mIsDirty = false;

		return blockId;
	}

	// Load an inode by its root block ID.
	protected LightInode
	load(int blockId)
		throws Throwable
	{
		checkSanity();
		// Read block type to understand how to load it.
		ByteBuffer buf = takeBuffer();
		buf.position(UNIT_SIZE);
		buf.flip();
		readBuffer(buf, blockId * BLOCK_SIZE);
		buf.flip();
		int type = buf.getInt();

		switch(type) {
		case BLOCK_TYPE_DIR: {
			LightInodeDir res = new LightInodeDir(this);
			res.mBlockId = blockId;
			res.load();
			return res;
		}
		case BLOCK_TYPE_FILE: {
			LightInodeFile res = new LightInodeFile(this);
			res.mBlockId = blockId;
			res.load();
			return res;
		}
		default: {
			throw new LightException.Corrupted(
				"Unknown block type");
		}
		}
	}

	///////////////////////////// Private API //////////////////////////////

	// Allocate a new block and append it to the free list.
	private void
	addBlock()
		throws Throwable
	{
		checkSanity();
		mIsDirty = true;
		ByteBuffer buf = takeBuffer();
		buf.put((byte)(mIsDirty ? 1 : 0));
		buf.flip();
		writeBuffer(buf, mIsDirtyPos);

		// Allocate a new block pointing at the current free list head.
		int blockId = mBlockCountTotal + 1;
		int offset = blockId * BLOCK_SIZE;
		buf = takeBuffer();
		buf.putInt(mNextFreeBlock);
		buf.position(BLOCK_SIZE);
		buf.flip();
		writeBuffer(buf, offset);

		// Make the new block the head.
		mNextFreeBlock = blockId;
		buf = takeBuffer();
		buf.putInt(mNextFreeBlock);
		buf.flip();
		writeBuffer(buf, mNextFreeBlockPos);

		++mBlockCountTotal;
		buf = takeBuffer();
		buf.putInt(mBlockCountTotal);
		buf.flip();
		writeBuffer(buf, mBlockCountTotalPos);

		++mBlockCountFree;
		buf = takeBuffer();
		buf.putInt(mBlockCountFree);
		buf.flip();
		writeBuffer(buf, mBlockCountFreePos);

		buf = takeBuffer();
		buf.put((byte)0);
		buf.flip();
		writeBuffer(buf, mIsDirtyPos);
		mIsDirty = false;
	}

	// Ensure there is at least one free block.
	private void
	reserveBlock()
		throws Throwable
	{
		if (mBlockCountFree == 0)
			addBlock();
	}

	// Create a new block manager. Wipe whatever there was before out.
	private void
	openNew()
		throws Throwable
	{
		ByteBuffer buf = takeBuffer();
		assert(buf.position() == mIsDirtyPos);
		buf.put((byte)(mIsDirty ? 1 : 0));

		assert(buf.position() == mBlockCountTotalPos);
		buf.putInt(mBlockCountTotal);

		assert(buf.position() == mBlockCountFreePos);
		buf.putInt(mBlockCountFree);

		assert(buf.position() == mNextFreeBlockPos);
		buf.putInt(mNextFreeBlock);

		buf.position(BLOCK_SIZE);
		buf.flip();
		writeBuffer(buf, 0);
	}

	// Load an existing LightFS block manager from a file.
	private void
	openExisting()
		throws Throwable
	{
		ByteBuffer buf = takeBuffer();
		buf.position(BLOCK_SIZE);
		buf.flip();
		readBuffer(buf, 0);
		buf.flip();

		assert(buf.position() == mIsDirtyPos);
		mIsDirty = buf.get() != 0;

		assert(buf.position() == mBlockCountTotalPos);
		mBlockCountTotal = buf.getInt();

		assert(buf.position() == mBlockCountFreePos);
		mBlockCountFree = buf.getInt();

		assert(buf.position() == mNextFreeBlockPos);
		mNextFreeBlock = buf.getInt();

		checkSanity();
	}

	private ByteBuffer mBuf;
	private RandomAccessFile mFile;
	private FileChannel mChan;

	private static final int mIsDirtyPos = 0;
	private boolean mIsDirty;

	private static final int mBlockCountTotalPos = 1;
	private int mBlockCountTotal;

	private static final int mBlockCountFreePos =
		mBlockCountTotalPos + UNIT_SIZE;
	private int mBlockCountFree;

	private static final int mNextFreeBlockPos =
		mBlockCountFreePos + UNIT_SIZE;
	private int mNextFreeBlock;
}
