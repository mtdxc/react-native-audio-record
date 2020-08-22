package com.goodatlas.audiorecord;

public class AudioConvert {
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    public static native long WavNew(String path, int samplerate, int channel);
    public static native long WavOpen(String path);
    public static native int WavGetSampleRate(long h);
    public static native int WavGetChannels(long h);
    public static native short[] WavRead(long h, int samples);
    public static native int WavWrite(long h, short[] pcm);
    public static native boolean WavClose(long h);

    public static native long Mp3Open(int samplerate, int channels, int bitrate);
    public static native int Mp3SamplePerPass(long h);
    public static native byte[] Mp3Encode(long h, short[] pcm);
    public static native boolean Mp3Close(long h);

    public static native int WavToMp3(String wavPath, String mp3Path, int bitrate);
    public static native int Mp3ToWav(String mp3Path, String wavPath);
}
