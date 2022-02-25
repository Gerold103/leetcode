package tests;

import lightfs.*;

public class Tests {

	public static void
	main(String[] args)
		throws Throwable
	{
		testOpenClose();
		testReadWrite();
		testPermissions();
		testList();
		testDeleteFile();
		testDeleteDir();
	}

	private static void
	check_equal(String[] got, String[] expected)
		throws Throwable
	{
		if (got.length != expected.length)
			throw new Throwable("Length mismatch");
		for (int i = 0; i < got.length; ++i) {
			if (!got[i].equals(expected[i]))
				throw new Throwable("Item mismatch");
		}
	}

	private static void
	testOpenClose()
		throws Throwable
	{
		System.out.println("Test open and close");

		String fsName = "test1.lfs";
		LightFileSystem fs = new LightFileSystem(fsName);
		fs.createFile("/file1");
		fs.createFile("/file2");
		fs.createDir("/dir1");
		fs.createFile("/dir1/file3");
		String[] rootEntries = new String[]{"file1", "file2", "dir1"};
		check_equal(fs.list("/", 0), rootEntries);
		fs.close();

		fs = new LightFileSystem(fsName);
		String[] entries = fs.list("/", 0);
		check_equal(entries, rootEntries);
		fs.close();
	}

	private static void
	testReadWrite()
		throws Throwable
	{
		System.out.println("Test read and write");

		String fsName = "test2.lfs";
		LightFileSystem fs = new LightFileSystem(fsName);
		fs.createDir("/dir1");
		fs.createDir("/dir1/dir2");
		String filePath = "/dir1/dir2/file";
		fs.createFile(filePath);
		LightFileWriteStream ws = fs.openFileWrite(filePath);

		String text = "test text";
		byte[] data = text.getBytes();
		ws.write(data);
		ws.close();

		LightFileReadStream rs = fs.openFileRead(filePath);
		data = new byte[data.length];

		int len = rs.read(data);
		assert(len == data.length);
		String result = new String(data);
		rs.close();
		assert(result.equals(text));

		ws = fs.openFileWrite(filePath);
		text = "12345";
		data = text.getBytes();
		ws.write(data);
		ws.close();

		text = "12345text";
		rs = fs.openFileRead(filePath);
		data = new byte[text.getBytes().length];
		len = rs.read(data);
		assert(len == data.length);
		result = new String(data);
		rs.close();
		assert(result.equals(text));

		len = 600;
		data = new byte[len];
		for (int i = 0; i < len; ++i)
			data[i] = (byte)i;
		ws = fs.openFileWrite(filePath);
		ws.write(data);
		ws.close();
		for (int i = 0; i < len; ++i)
			data[i] = 0;

		rs = fs.openFileRead(filePath);
		int lenRet = rs.read(data);
		assert(lenRet == len);
		rs.close();
		for (int i = 0; i < len; ++i)
			assert(data[i] == (byte)i);

		ws = fs.openFileWrite(filePath);
		ws.write("xxxxxxxxx".getBytes());
		ws.setPosition(3);
		ws.write("01234".getBytes());
		ws.close();

		text = "xxx01234x";
		rs = fs.openFileRead(filePath);
		data = new byte[text.getBytes().length];
		len = rs.read(data);
		assert(len == data.length);
		result = new String(data);
		rs.close();
		assert(result.equals(text));

		fs.close();
	}

	private static void
	testPermissions()
		throws Throwable
	{
		System.out.println("Test permissions");

		String fsName = "test3.lfs";
		String filePath = "/file";
		LightFileSystem fs = new LightFileSystem(fsName);
		fs.createFile(filePath);

		LightFileWriteStream ws = fs.openFileWrite(filePath);
		ws.write("123".getBytes());

		LightFileReadStream rs;
		try {
			rs = fs.openFileRead(filePath);
		} catch (LightException.Busy ex) {
		} catch (Throwable ex) {
			assert(false);
		}

		ws.close();
		rs = fs.openFileRead(filePath);
		try {
			ws = fs.openFileWrite(filePath);
		} catch (LightException.Busy ex) {
		} catch (Throwable ex) {
			assert(false);
		}
		rs.close();

		fs.close();
	}

	private static void
	testList()
		throws Throwable
	{
		System.out.println("Test list()");

		String fsName = "test4.lfs";
		LightFileSystem fs = new LightFileSystem(fsName);

		fs.createFile("/file1");
		fs.createDir("/dir1");
		fs.createFile("/dir1/file2");
		fs.createFile("/dir1/file3");
		fs.createDir("/dir1/dir2");
		fs.createFile("/dir1/dir2/file4");
		fs.createFile("/dir1/dir2/file5");

		// List in the specified folder, non-recursive, all content.
		check_equal(fs.list("/", 0), new String[]{"file1", "dir1"});

		check_equal(fs.list("/dir1", 0), new String[]{
			"file2", "file3", "dir2"});

		check_equal(fs.list("/dir1/dir2", 0), new String[]{
			"file4", "file5"});

		// List recursive, all content.
		check_equal(fs.list("/", LightFileSystem.LIST_RECURSIVE),
			new String[]{"file1", "dir1", "dir1/file2",
			"dir1/file3", "dir1/dir2", "dir1/dir2/file4",
			"dir1/dir2/file5"});

		// List recursive, only files.
		check_equal(fs.list("/", LightFileSystem.LIST_RECURSIVE |
			LightFileSystem.LIST_ONLY_FILES), new String[]{
			"file1", "dir1/file2", "dir1/file3", "dir1/dir2/file4",
			"dir1/dir2/file5"});

		// List recursive, only dirs.
		check_equal(fs.list("/", LightFileSystem.LIST_RECURSIVE |
			LightFileSystem.LIST_ONLY_DIRS), new String[]{
			"dir1", "dir1/dir2"});

		fs.close();
	}

	private static void
	testDeleteFile()
		throws Throwable
	{
		System.out.println("Test file deletion");

		String fsName = "test5.lfs";
		LightFileSystem fs = new LightFileSystem(fsName);

		fs.createFile("/file1");
		fs.createDir("/dir1");
		fs.createFile("/dir1/file2");
		fs.createFile("/dir1/file3");
		fs.createDir("/dir1/dir2");
		fs.createFile("/dir1/dir2/file4");
		fs.createFile("/dir1/dir2/file5");
		check_equal(fs.list("/", 0), new String[]{"file1", "dir1"});
		fs.deleteFile("/file1");
		check_equal(fs.list("/", 0), new String[]{"dir1"});

		check_equal(fs.list("/dir1/dir2", 0), new String[]{
			"file4", "file5"});
		fs.deleteFile("/dir1/dir2/file4");
		check_equal(fs.list("/dir1/dir2", 0), new String[]{
			"file5"});

		fs.deleteFile("/dir1/dir2/file5");
		check_equal(fs.list("/dir1/dir2", 0), new String[]{});

		check_equal(fs.list("/dir1", 0), new String[]{
			"file2", "file3", "dir2"});
		fs.deleteFile("/dir1/file3");
		check_equal(fs.list("/dir1/", 0), new String[]{
			"file2", "dir2"});

		fs.deleteFile("/dir1/file2");
		check_equal(fs.list("/dir1/", 0), new String[]{"dir2"});

		fs.close();
	}

	private static void
	testDeleteDir()
		throws Throwable
	{
		System.out.println("Test directory deletion");

		String fsName = "test6.lfs";
		LightFileSystem fs = new LightFileSystem(fsName);

		fs.createFile("/file1");
		fs.createDir( "/dir1");
		fs.createFile("/dir1/file2");
		fs.createFile("/dir1/file3");
		fs.createDir( "/dir1/dir2");
		fs.createFile("/dir1/dir2/file4");
		fs.createFile("/dir1/dir2/file5");
		fs.createDir( "/dir1/dir3");
		fs.createFile("/dir1/dir3/file6");
		fs.createFile("/dir1/dir3/file7");
		fs.createDir( "/dir1/dir3/dir4");
		fs.createDir( "/dir1/dir3/dir5");

		fs.deleteDir("/dir1/dir3/dir4");
		check_equal(fs.list("/dir1/dir3", 0), new String[]{
			"file6", "file7", "dir5"});

		fs.deleteDir("/dir1/dir3");
		check_equal(fs.list("/dir1", 0), new String[]{
			"file2", "file3", "dir2"});

		fs.deleteDir("/dir1");
		check_equal(fs.list("/", 0), new String[]{"file1"});

		fs.close();
	}
}
