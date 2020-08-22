#include <string>
#include <vector>
#include <iostream>
extern "C" {
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"
#include "shine_mp3.h"
}

#ifdef WIN32
#include <Windows.h>
static std::wstring ToUtf16(const std::string& str) {
  std::wstring ret;
  DWORD nWide = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), NULL, 0);
  if (nWide) {
    wchar_t* wideFileName = new wchar_t[nWide + 1];
    wideFileName[nWide] = 0;
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), wideFileName, nWide);
    ret = wideFileName;
    delete[] wideFileName;
  }
  return ret;
}
#endif
static FILE* Utf8FileOpen(const char* path, const char* mode) {
#ifdef WIN32
  std::wstring wpath = ToUtf16(path);
  std::wstring wmode = ToUtf16(mode);
  return _wfopen(wpath.c_str(), wmode.c_str());
#else
  return fopen(path, mode);
#endif
}

bool WavClose(long h) {
  if (drwav* wav = (drwav*)h) {
    drwav_uninit(wav);
    delete wav;
  }
  return true;
}

long WavNew(const char* path, int samplerate, int channel) {
  drwav* wav = new drwav;
  drwav_data_format fmt;
  fmt.container = drwav_container_riff;
  fmt.format = DR_WAVE_FORMAT_PCM;
  fmt.channels = channel;
  fmt.sampleRate = samplerate;
  fmt.bitsPerSample = 16;
#ifdef WIN32
  if (!drwav_init_file_write_w(wav, ToUtf16(path).c_str(), &fmt, nullptr))
#else
  if (!drwav_init_file_write(wav, path, &fmt, nullptr))
#endif
  {
    std::cerr << "unable to init write" << path;
    //WavClose((long)wav);
    delete wav;
    wav = nullptr;
  }
  return (long)wav;
}

long WavOpen(const char* path) {
  drwav* wav = new drwav;
#ifdef WIN32
  if (!drwav_init_file_w(wav, ToUtf16(path).c_str(), nullptr))
#else
  if (!drwav_init_file(wav, path, nullptr))
#endif
  {
    std::cerr << "unable to init read" << path;
    delete wav;
    wav = nullptr;
  }
  return (long)wav;
}

int WavGetSampleRate(long h) {
  int ret = 0;
  if (drwav* wav = (drwav*)h) {
    ret = wav->sampleRate;
  }
  return ret;
}

int WavGetChannels(long h) {
  int ret = 0;
  if (drwav* wav = (drwav*)h) {
    ret = wav->channels;
  }
  return ret;
}

int WavRead(long h, int16_t* pcm, int samples) {
  int ret = 0;
  if (drwav* wav = (drwav*)h) {
    ret = drwav_read_pcm_frames_s16(wav, samples, pcm);
  }
  return ret;
}

int WavWrite(long h, int16_t* pcm, int samples) {
  int ret = 0;
  if (drwav* wav = (drwav*)h) {
    ret = drwav_write_pcm_frames(wav, samples, pcm);
  }
  return ret;
}

long Mp3Open(int samplerate, int channels, int bitrate) {
  shine_config_t config;
  shine_set_config_mpeg_defaults(&config.mpeg);
  config.wave.samplerate = samplerate;
  if (channels > 1) {
    config.mpeg.mode = STEREO;
    config.wave.channels = PCM_STEREO;
  }
  else {
    config.mpeg.mode = MONO;
    config.wave.channels = PCM_MONO;
  }
  config.mpeg.bitr = bitrate / 1000;
  if (shine_check_config(config.wave.samplerate, config.mpeg.bitr) < 0) {
    std::cerr << "Unsupported samplerate/bitrate configuration.";
    return false;
  }
  return (long)shine_initialise(&config);
}

int Mp3SamplePerPass(long h) {
  shine_t enc = (shine_t)h;
  if (!enc) return 0;
  return shine_samples_per_pass(enc);
}

uint8_t* Mp3Encode(long h, int16_t* pcm, int len, int* outlen) {
  shine_t enc = (shine_t)h;
  if (pcm) {
    return shine_encode_buffer_interleaved(enc, pcm, outlen);
  }
  else {
    return shine_flush(enc, outlen);
  }
}

bool Mp3Close(long h) {
  if(shine_t enc = (shine_t)h)
    shine_close(enc);
  return true;
}

int WavToMp3(const char* wavPath, const char* mp3Path, int bitrate) {
  int ret = 0;
  long wav = WavOpen(wavPath);
  if(!wav){
    std::cerr << "unable to open wavFile " << wavPath;
    return ret;
  }
  FILE* fp = Utf8FileOpen(mp3Path, "wb");
  if (!fp) {
    std::cerr << "unable to open mp3File " << wavPath;
    WavClose(wav);
    return ret;
  }
  int channel = WavGetChannels(wav);
  long enc = Mp3Open(WavGetSampleRate(wav), channel, bitrate);
  int samples_pre_pass = Mp3SamplePerPass(enc);
  int mp3_size; uint8_t* mp3_buf;
  std::vector<int16_t> pcm(samples_pre_pass * channel);
  while (enc) {
    int n = WavRead(wav, &pcm[0], samples_pre_pass);
    if (!n) {
      mp3_buf = Mp3Encode(enc, NULL, 0, &mp3_size);
      if (mp3_buf && mp3_size) {
        fwrite(mp3_buf, 1, mp3_size, fp);
      }
      break;
    }
    ret += n;
    mp3_buf = Mp3Encode(enc, &pcm[0], pcm.size(), &mp3_size);
    if (mp3_buf && mp3_size) {
      fwrite(mp3_buf, 1, mp3_size, fp);
    }
  }
  WavClose(wav);
  Mp3Close(enc);
  if (fp) {
    fclose(fp);
    fp = nullptr;
  }
  return ret;
}

// Mp3压缩比大概12：1
#define INBUF_SIZE 1152
int Mp3ToWav(const char* mp3Path, const char* wavPath) {
  if (!wavPath || !wavPath[0] || !mp3Path || !mp3Path[0]) {
    std::cerr << "path args error";
    return 0;
  }

  drmp3 mp3;
#ifdef WEBRTC_WIN
  if (!drmp3_init_file_w(&mp3, ToUtf16(mp3Path).c_str(), NULL))
#else
  if (!drmp3_init_file(&mp3, mp3Path, NULL))
#endif
  {
    // Failed to open file
    std::cerr << "drmp3_init failed " << mp3Path;
    return 0;
  }
  int channels = mp3.channels;
  long wav = WavNew(wavPath, mp3.sampleRate, channels);
  int total_samples = 0;
  short pcm[INBUF_SIZE * 10];
  while (wav) {
    drmp3_uint64 framesRead = drmp3_read_pcm_frames_s16(
        &mp3, sizeof(pcm) / sizeof(pcm[0]) / channels, pcm);
    if (framesRead <= 0)
      break;
    WavWrite(wav, pcm, framesRead);
    total_samples += framesRead;
  }
  drmp3_uninit(&mp3);
  WavClose(wav);
  return total_samples;
}