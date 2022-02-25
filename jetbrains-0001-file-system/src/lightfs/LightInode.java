package lightfs;

// Represents en entity stored in LightFS: file or a directory.
//
public class LightInode
{
	protected
	LightInode()
	{
		mType = 0;
		mBlockId = 0;
		mReadCount = 0;
		mIsLoaded = false;
		mIsWriting = false;
	}

	protected static final int mTypePos = 0;
	protected int mType;
	protected int mBlockId;
	protected int mReadCount;
	protected boolean mIsLoaded;
	protected boolean mIsWriting;
	protected String mName;
}
