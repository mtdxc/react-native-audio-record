#include <jni.h>
#include <string>
#include "../../../../convert/convert.h"
// Given a (UTF-16) jstring return a new UTF-8 native string.
std::string JavaToStdString(JNIEnv* jni, const jstring& j_string) {
    if(j_string==NULL) return std::string();
    const char* chars = jni->GetStringUTFChars(j_string, NULL);
    //CHECK_EXCEPTION(jni) << "Error during GetStringUTFChars";
    std::string str(chars, jni->GetStringUTFLength(j_string));
    //CHECK_EXCEPTION(jni) << "Error during GetStringUTFLength";
    jni->ReleaseStringUTFChars(j_string, chars);
    //CHECK_EXCEPTION(jni) << "Error during ReleaseStringUTFChars";
    return str;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_goodatlas_audiorecord_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}


extern "C" JNIEXPORT jlong JNICALL
Java_com_goodatlas_audiorecord_AudioConvert_WavNew(
        JNIEnv* env, jclass cls,
        jstring wav, int samplerate, int channel) {
    std::string path = JavaToStdString(env, wav);
    return WavNew(path.c_str(), samplerate, channel);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_goodatlas_audiorecord_AudioConvert_WavOpen(
        JNIEnv* env, jclass cls,
        jstring wav) {
    std::string path = JavaToStdString(env, wav);
    return WavOpen(path.c_str());
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_goodatlas_audiorecord_AudioConvert_WavClose(
        JNIEnv* env, jclass cls,
        jlong wav) {
    return WavClose(wav);
}

extern "C" JNIEXPORT int JNICALL
Java_com_goodatlas_audiorecord_AudioConvert_WavGetSampleRate(
        JNIEnv* env, jclass cls,jlong wav) {
    return WavGetSampleRate(wav);
}

extern "C" JNIEXPORT int JNICALL
Java_com_goodatlas_audiorecord_AudioConvert_WavGetChannels(
        JNIEnv* env, jclass cls, jlong wav) {
    return WavGetChannels(wav);
}

extern "C" JNIEXPORT jshortArray JNICALL
Java_com_goodatlas_audiorecord_AudioConvert_WavRead(
        JNIEnv* env, jclass cls,
        jlong wav, int size) {
    int sample_per_channel = size / WavGetChannels(wav);
    jshortArray ret = env->NewShortArray(size);
    jshort* pcm = env->GetShortArrayElements(ret, nullptr);
    WavRead(wav, pcm, sample_per_channel);
    env->ReleaseShortArrayElements(ret, pcm, 0);
    return ret;
}

extern "C" JNIEXPORT int JNICALL
Java_com_goodatlas_audiorecord_AudioConvert_WavWrite(
        JNIEnv* env, jclass cls,
        jlong wav, jshortArray jary) {
    jshort* pcm = env->GetShortArrayElements(jary, nullptr);
    int samples = env->GetArrayLength(jary) / WavGetChannels(wav);
    int ret = WavRead(wav, pcm, samples);
    env->ReleaseShortArrayElements(jary, pcm, 0);
    return ret;// * WavGetChannels(wav);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_goodatlas_audiorecord_AudioConvert_Mp3Open(
        JNIEnv* env, jclass cls,
        int samplerate, int channels, int bitrate) {
    return Mp3Open(samplerate, channels, bitrate);
}

extern "C" JNIEXPORT int JNICALL
Java_com_goodatlas_audiorecord_AudioConvert_Mp3SamplePerPass(JNIEnv* env, jclass cls, jlong enc) {
    return Mp3SamplePerPass(enc) * Mp3GetChannels(enc);
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_goodatlas_audiorecord_AudioConvert_Mp3Encode(JNIEnv* env, jclass cls,
        jlong enc, jshortArray jpcm) {
    jbyteArray ret = nullptr;
    short* pcm = env->GetShortArrayElements(jpcm, nullptr);
    int len = env->GetArrayLength(jpcm), outlen = 0;
    //ASSERT(len == Mp3SamplePerPass(enc) * Mp3GetChannels(enc));
    uint8_t* mp3 = Mp3Encode(enc, pcm, &outlen);
    env->ReleaseShortArrayElements(jpcm, pcm, 0);
    if(mp3 && outlen){
        ret = env->NewByteArray(outlen);
        jbyte* dst = env->GetByteArrayElements(ret, nullptr);
        memcpy(dst, mp3, outlen);
        env->ReleaseByteArrayElements(ret, dst, 0);
    }
    return ret;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_goodatlas_audiorecord_AudioConvert_Mp3Close(JNIEnv* env, jclass cls, jlong enc) {
    return Mp3Close(enc);
}

extern "C" JNIEXPORT int JNICALL
Java_com_goodatlas_audiorecord_AudioConvert_WavToMp3(
        JNIEnv* env, jclass cls,
        jstring jwav,jstring jmp3, int bitrate) {
    std::string wav = JavaToStdString(env, jwav);
    std::string mp3 = JavaToStdString(env, jmp3);
    return WavToMp3(wav.c_str(), mp3.c_str(), bitrate);
}

extern "C" JNIEXPORT int JNICALL
Java_com_goodatlas_audiorecord_AudioConvert_Mp3ToWav(
        JNIEnv* env, jclass cls,
        jstring jmp3,jstring jwav) {
    std::string wav = JavaToStdString(env, jwav);
    std::string mp3 = JavaToStdString(env, jmp3);
    return Mp3ToWav(mp3.c_str(), wav.c_str());
}
