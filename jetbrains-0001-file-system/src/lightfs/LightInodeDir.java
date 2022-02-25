package lightfs;

import java.nio.ByteBuffer;
import java.util.ArrayList;

import static lightfs.LightUtils.*;

// LightFS directory inode.
//
public class LightInodeDir extends LightInode
{
	protected
	LightInodeDir(LightBlockManager manager)
	{
		mType = BLOCK_TYPE_DIR;
		mManager = manager;
		mChildren = new LightInode[DIR_CAPACITY];
	}

	// Write the directory into its block as a new entity, when it is just
	// created.
	protected void
	writeAsNew()
		throws Throwable
	{
		assert(mBlockId > 0);
		assert(mReadCount == 0);
		assert(!mIsWriting);
		// New dir can't have any children.
		for (LightInode i : mChildren)
			assert(i == null);

		mManager.checkSanity();
		ByteBuffer buf = mManager.takeBuffer();

		assert(buf.position() == mTypePos);
		buf.putInt(mType);

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

	// Load the directory from its block. Non-recursive, children are not
	// loaded until needed.
	protected void
	load()
		throws Throwable
	{
		assert(mBlockId > 0);
		assert(mReadCount == 0);
		assert(!mIsWriting);
		if (mIsLoaded)
			return;

		mManager.checkSanity();
		ByteBuffer buf = mManager.takeBuffer();
		buf.position(BLOCK_SIZE);
		buf.flip();
		mManager.readBuffer(buf, mBlockId * BLOCK_SIZE);
		buf.flip();

		assert(buf.position() == mTypePos);
		mType = buf.getInt();
		assert(mType == BLOCK_TYPE_DIR);

		assert(buf.position() == mNameLenPos);
		int nameLen = buf.getInt();
		assert(nameLen <= MAX_NAME_SIZE);
		assert(nameLen > 0);

		int pos = buf.position();
		assert(pos == mNamePos);
		byte[] nameRaw = new byte[nameLen];
		buf.get(nameRaw);
		mName = new String(nameRaw);
		// Skip name padding.
		buf.position(pos + MAX_NAME_SIZE);

		assert(buf.position() == mChildrenPos);
		for (int i = 0; i < DIR_CAPACITY; ++i) {
			int blockId = buf.getInt();
			if (blockId == 0)
				continue;
			LightInode child = new LightInode();
			child.mBlockId = blockId;
			mChildren[i] = child;
		}
		mIsLoaded = true;
	}

	// Find a child inode with the given name. Non-recursive.
	protected LightInode
	findChild(String name)
		throws Throwable
	{
		int idx = findChildIdx(name);
		if (idx < 0)
			return null;
		return mChildren[idx];
	}

	// Create a file in the directory.
	protected void
	createFile(String name)
		throws Throwable
	{
		load();
		LightInode baseChild = findChild(name);
		if (baseChild != null)
			throw new LightException.Duplicate("file");
		int cellId = findChildFreeIdx();
		if (cellId == -1)
			throw new LightException.TooBig("dir");

		LightInodeFile child = new LightInodeFile(mManager);
		child.mName = name;
		child.mBlockId = mManager.takeBlock();
		child.writeAsNew();

		addChild(cellId, child);
	}

	// Create a subdirectory in this one.
	protected void
	createDir(String name)
		throws Throwable
	{
		load();
		LightInode baseChild = findChild(name);
		if (baseChild != null)
			throw new LightException.Duplicate("dir");
		int cellId = findChildFreeIdx();
		if (cellId == -1)
			throw new LightException.TooBig("dir");

		LightInodeDir child = new LightInodeDir(mManager);
		child.mName = name;
		child.mBlockId = mManager.takeBlock();
		child.writeAsNew();

		addChild(cellId, child);
	}

	// Delete a file from the directory. With all its content.
	protected void
	deleteFile(String name)
		throws Throwable
	{
		load();
		int idx = findChildIdx(name);
		if (idx < 0)
			throw new LightException.NotFound("file");
		LightInode child = mChildren[idx];
		if (child.mType != BLOCK_TYPE_FILE)
			throw new LightException.InvalidArgs("not a file");
		if (child.mReadCount > 0)
			throw new LightException.Busy("ready");
		if (child.mIsWriting)
			throw new LightException.Busy("write");

		removeChild(idx);
		((LightInodeFile)child).unlink();
	}

	// Delete a subdirectory from this one, recursively, with all its
	// content.
	protected void
	deleteDir(String name)
		throws Throwable
	{
		load();
		int idx = findChildIdx(name);
		if (idx < 0)
			throw new LightException.NotFound("dir");
		LightInode child = mChildren[idx];
		if (child.mType != BLOCK_TYPE_DIR)
			throw new LightException.InvalidArgs("not a dir");
		if (child.mReadCount > 0)
			throw new LightException.Busy("ready");
		if (child.mIsWriting)
			throw new LightException.Busy("write");

		removeChild(idx);
		((LightInodeDir)child).unlink();
	}

	// Delete the directory and all its content, recursively, with all its
	// content.
	protected void
	unlink()
		throws Throwable
	{
		assert(mBlockId > 0);
		assert(mIsLoaded);
		mManager.checkSanity();
		for (int i = 0; i < DIR_CAPACITY; ++i) {
			LightInode child = mChildren[i];
			if (child == null)
				continue;
			if (child.mType == BLOCK_TYPE_DIR)
				deleteDir(child.mName);
			else if (child.mType == BLOCK_TYPE_FILE)
				deleteFile(child.mName);
			else
				assert(false);
			assert(mChildren[i] == null);
		}
		mManager.putBlock(mBlockId);
		mBlockId = 0;
		mIsLoaded = false;
	}

	// Get names of all children matching the flags. Recursively, if
	// requested. In case of recursion the non-direct children's names are
	// returned with a path to them as a prefix.
	protected ArrayList<String>
	list(String prefix, int flags)
		throws Throwable
	{
		load();
		ArrayList<String> list = new ArrayList<String>();
		for (int i = 0; i < DIR_CAPACITY; ++i) {
			LightInode inode = mChildren[i];
			if (inode == null)
				continue;

			if (!inode.mIsLoaded) {
				inode = mManager.load(inode.mBlockId);
				mChildren[i] = inode;
			}
			boolean isNeeded = true;
			if (inode.mType == BLOCK_TYPE_DIR) {
				isNeeded = (flags & LIST_ONLY_FILES) == 0;
			} else {
				assert(inode.mType == BLOCK_TYPE_FILE);
				isNeeded = (flags & LIST_ONLY_DIRS) == 0;
			}
			if (isNeeded)
				list.add(prefix + inode.mName);

			if (inode.mType == BLOCK_TYPE_DIR &&
			(flags & LIST_RECURSIVE) != 0) {
				LightInodeDir dir = (LightInodeDir)inode;
				String nextPrefix = prefix + dir.mName + "/";
				list.addAll(dir.list(nextPrefix, flags));
			}
		}
		return list;
	}

	///////////////////////////// Private API //////////////////////////////

	// Find index of a child with the given name.
	private int
	findChildIdx(String name)
		throws Throwable
	{
		load();
		for (int i = 0; i < DIR_CAPACITY; ++i) {
			LightInode child = mChildren[i];
			if (child == null)
				continue;
			if (child.mName.equals(name))
				return i;
		}
		return -1;
	}

	// Find an unused child index.
	private int
	findChildFreeIdx()
		throws Throwable
	{
		load();
		for (int i = 0; i < DIR_CAPACITY; ++i) {
			if (mChildren[i] == null)
				return i;
		}
		return -1;
	}

	// Detach the child with the given index. It is not deleted, only
	// detached.
	private void
	removeChild(int idx)
		throws Throwable
	{
		ByteBuffer buf = mManager.takeBuffer();
		int offset = mBlockId * BLOCK_SIZE;
		buf.putInt(0);
		buf.flip();
		mManager.writeBuffer(buf, offset + mChildrenPos +
			idx * UNIT_SIZE);
		mChildren[idx] = null;
	}

	// Attach the child with the given index. Should already be created.
	private void
	addChild(int idx, LightInode child)
		throws Throwable
	{
		assert(mChildren[idx] == null);
		mChildren[idx] = child;
		int offset = mBlockId * BLOCK_SIZE + mChildrenPos;
		offset += idx * UNIT_SIZE;

		ByteBuffer buf = mManager.takeBuffer();
		buf.putInt(child.mBlockId);
		buf.flip();
		mManager.writeBuffer(buf, offset);
	}

	protected static final int mNameLenPos = mTypePos + UNIT_SIZE;
	protected static final int mNamePos = mNameLenPos + UNIT_SIZE;
	protected static final int mChildrenPos = mNamePos + MAX_NAME_SIZE;
	protected LightBlockManager mManager;
	protected LightInode[] mChildren;
}
