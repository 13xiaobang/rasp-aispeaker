package ai.speaker;

public class AiSpeakerManager{
	private static final String TAG = "AiSpeakerManager";
	private static volatile AiSpeakerManager defaultInstance;
    static {
		System.loadLibrary("aispeakermgr_jni");
	}
	public AiSpeakerManager() {}
	public static AiSpeakerManager getInstance() {
        if (defaultInstance == null) {
            synchronized (AiSpeakerManager.class) {
                if (defaultInstance == null) {
                    defaultInstance = new AiSpeakerManager();
                }
            }
        }
        return defaultInstance;
    }
	/*
	//public api:
	public native int setMode(int mode);
	public native int getMode();
	public native String getStr();
	public native byte[] getPcm();//temp use string to test blob r/w
	public native int pollEvent();
	*/

	// official interface:
	public native int enquiry(); //  return max size
	public native int open();
	public native int read(byte[] javaAudioData, int sizeInBytes); // return read size.
	public native int close();
}
