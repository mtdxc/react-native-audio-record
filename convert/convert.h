#ifndef AUDIO_CONVERT_H
#define AUDIO_CONVERT_H

long WavNew(const char* path, int samplerate, int channel);
long WavOpen(const char* path);
int WavGetSampleRate(long h);
int WavGetChannels(long h);
int WavRead(long h, short* pcm, int samples_pre_chanel);
int WavWrite(long h, short* pcm, int samples_pre_chanel);
bool WavClose(long h);

long Mp3Open(int samplerate, int channels, int bitrate);
int Mp3SamplePerPass(long h);
int Mp3GetChannels(long h);
unsigned char* Mp3Encode(long h, short* pcm, int* outlen);
bool Mp3Close(long h);

int WavToMp3(const char* wavPath, const char* mp3Path, int bitrate);
int Mp3ToWav(const char* mp3Path, const char* wavPath);

#endif