package lightfs;

import java.util.ArrayList;

import static lightfs.LightUtils.*;

// LightFS is a very simple toy file system stored in a single file on a real
// file system.
//
// It supports file hierarchy with directories, multiple read-streams on the
// same file. Not much else. Its main feature is simplicity (on the edge of
// stupidity though).
//
// Limitations:
// - No multithreading;
// - No multiple write-streams on the same file;
// - File size and file count in a directory are very limited;
// - Inodes are not unloaded once they are loaded. It means in the worst case
//   all metadata is stored in memory;
// - Data blocks are not cached by LightFS (although probably are by the OS);
// - Incomplete test coverage - some corner cases are not tested.
//
// On the FS structure see the block manager doc.
//
public class LightFileSystem
{
	// The flags to configure behaviour of list().
	public static final int LIST_RECURSIVE = LightUtils.LIST_RECURSIVE;
	public static final int LIST_ONLY_FILES = LightUtils.LIST_ONLY_FILES;
	public static final int LIST_ONLY_DIRS = LightUtils.LIST_ONLY_DIRS;

	// Create or open a LightFS. If the specified file doesn't exist, then a
	// new one is created. Otherwise tries to open the existing one.
	public
	LightFileSystem(String path)
		throws Throwable
	{
		mManager = new LightBlockManager(path);
		if (mManager.blockCount() == 0) {
			mRoot = new LightInodeDir(mManager);
			mRoot.mBlockId = mManager.takeBlock();
			mRoot.mName = "/";
			mRoot.writeAsNew();
		} else {
			LightInode root = mManager.load(BLOCK_FIRST_ID);
			if (root.mType != BLOCK_TYPE_DIR ||
			    !root.mName.equals("/")) {
				throw new LightException.Corrupted(
					"Bad root block");
			}
			mRoot = (LightInodeDir)root;
		}
	}

	// Close FS. The object becomes unusable after that.
	public void
	close()
		throws Throwable
	{
		mRoot = null;
		mManager.close();
		mManager = null;
	}

	// Create a new file. If it already exists, then an exception is thrown.
	// The path to the file must exist.
	public void
	createFile(String path)
		throws Throwable
	{
		String[] parts = splitPath(path);
		assert(path.charAt(0) == '/');
		assert(parts.length > 0);
		LightInodeDir dir = followPathTo(parts);
		dir.createFile(parts[parts.length - 1]);
	}

	// Create a new directory. If it already exists, then an exception is
	// thrown. The path to the directory must exist.
	public void
	createDir(String path)
		throws Throwable
	{
		String[] parts = splitPath(path);
		assert(path.charAt(0) == '/');
		assert(parts.length > 0);
		LightInodeDir dir = followPathTo(parts);
		dir.createDir(parts[parts.length - 1]);
	}

	// Delete a file. If it doesn't exist, then an exception is thrown.
	public void
	deleteFile(String path)
		throws Throwable
	{
		String[] parts = splitPath(path);
		assert(path.charAt(0) == '/');
		assert(parts.length > 0);
		LightInodeDir dir = followPathTo(parts);
		dir.deleteFile(parts[parts.length - 1]);
	}

	// Delete a directory recursively, including all its content. If the dir
	// doesn't exist, then an exception is thrown.
	public void
	deleteDir(String path)
		throws Throwable
	{
		String[] parts = splitPath(path);
		assert(path.charAt(0) == '/');
		assert(parts.length > 0);
		LightInodeDir dir = followPathTo(parts);
		dir.deleteDir(parts[parts.length - 1]);
	}

	// Open a file for writing. If the path to it doesn't exist or it is not
	// a file, or it is used for reading, then an exception is thrown. Only
	// one write-stream can be opened on the same file.
	public LightFileWriteStream
	openFileWrite(String path)
		throws Throwable
	{
		String[] parts = splitPath(path);
		assert(path.charAt(0) == '/');
		assert(parts.length > 0);
		LightInodeDir dir = followPathTo(parts);
		LightInode file = dir.findChild(parts[parts.length - 1]);
		return new LightFileWriteStream(file, mManager);
	}

	// Open a file for reading. If the path to it doesn't exist or it is not
	// a file, or it is used for writing, then an exception is thrown.
	// Multiple read-streams can be opened on the same file.
	public LightFileReadStream
	openFileRead(String path)
		throws Throwable
	{
		String[] parts = splitPath(path);
		assert(path.charAt(0) == '/');
		assert(parts.length > 0);
		LightInodeDir dir = followPathTo(parts);
		LightInode file = dir.findChild(parts[parts.length - 1]);
		return new LightFileReadStream(file, mManager);
	}

	// Get all entries in a directory. Of which types and whether should be
	// recursive - it is controlled by the flags. See LIST_* flags.
	public String[]
	list(String path, int flags)
		throws Throwable
	{
		String[] parts = splitPath(path);
		assert(path.charAt(0) == '/');
		LightInodeDir dir = followPath(parts);
		ArrayList<String> list = dir.list("", flags);
		String[] res = new String[list.size()];
		list.toArray(res);
		return res;
	}

	///////////////////////////// Private API //////////////////////////////

	// Follow path to the last part not including it. It is useful when
	// want to fetch the last part in a special way.
	private LightInodeDir
	followPathTo(String[] parts)
		throws Throwable
	{
		return followPathByCount(parts, parts.length - 1);
	}

	// Follow the path to the last part.
	private LightInodeDir
	followPath(String[] parts)
		throws Throwable
	{
		return followPathByCount(parts, parts.length);
	}

	private LightInodeDir
	followPathByCount(String[] parts, int count)
		throws Throwable
	{
		LightInodeDir dir = mRoot;
		for (int i = 0; i < count; ++i) {
			dir.load();
			LightInode next = dir.findChild(parts[i]);
			if (next == null)
				throw new LightException.NotFound("path part");
			if (next.mType != BLOCK_TYPE_DIR) {
				throw new LightException.InvalidArgs(
					"Not a path");
			}
			dir = (LightInodeDir)next;
		}
		return dir;
	}

	private LightBlockManager mManager;
	private LightInodeDir mRoot;
}
