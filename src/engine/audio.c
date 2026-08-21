#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio.h"
#include "dependencies/miniaudio.h"

#define OS_BELL_FREQUENCY 985.0
#define OS_BELL_AMPLITUDE 0.4
#define OS_BELL_DURATION_SECONDS 0.1

#define OS_ENGINE_CLICK_BASE_FREQUENCY 255.0
#define OS_ENGINE_CLICK_FREQUENCY_PER_SPEED 81.0
#define OS_ENGINE_CLICK_RATE_BASE 12.2
#define OS_ENGINE_CLICK_RATE_PER_SPEED 1.9

#define OS_FIRE_CLOCK_HZ 1020484.0
#define OS_FIRE_NOISE_CLICK_COUNT 256
#define OS_FIRE_TONE_STEP_COUNT 255
#define OS_FIRE_AMPLITUDE 0.3

#define OS_AUDIO_BANK_PATH "audio.bin"
#define OS_AUDIO_BANK_MAGIC "OSAB"
#define OS_AUDIO_CLIP_NAME_SIZE 32
#define OS_AUDIO_DIRECTORY_ENTRY_SIZE (OS_AUDIO_CLIP_NAME_SIZE + 4 + 4 + 4 + 8 + 8)
#define OS_AUDIO_HEADER_SIZE 12

static ma_engine engine;
static ma_waveform bellWaveform;
static ma_sound bellSound;

static ma_audio_buffer_ref engineClickRef;
static ma_sound engineHumSound;
static int engineHumActive = 0;
static double engineHumSpeedModifier = 1.0;
static double engineHumTimer = 0.0;

static float *firePCM = NULL;
static ma_uint32 fireFrameCount = 0;
static ma_audio_buffer_ref fireBufferRef;
static ma_sound fireSound;

static unsigned char *bankData = NULL;
static ma_uint32 bankClipCount = 0;

static ma_audio_buffer_ref alertClipRef;
static ma_sound alertSound;
static double alertClipDurationSeconds = 0.0;

static int initialized = 0;

static ma_uint32 audio_readU32LE(const unsigned char *p) {
  return (ma_uint32) p[0] | ((ma_uint32) p[1] << 8) | ((ma_uint32) p[2] << 16) | ((ma_uint32) p[3] << 24);
}

static ma_uint64 audio_readU64LE(const unsigned char *p) {
  ma_uint64 lo = audio_readU32LE(p);
  ma_uint64 hi = audio_readU32LE(p + 4);
  return lo | (hi << 32);
}

static const unsigned char *audio_findClipEntry(const char *name) {
  for (ma_uint32 i = 0; i < bankClipCount; i++) {
    const unsigned char *entry = bankData + OS_AUDIO_HEADER_SIZE + (ma_uint64) i * OS_AUDIO_DIRECTORY_ENTRY_SIZE;
    if (strncmp((const char*) entry, name, OS_AUDIO_CLIP_NAME_SIZE) == 0) {
      return entry;
    }
  }
  return NULL;
}

static int audio_loadBank() {
  FILE *file = fopen(OS_AUDIO_BANK_PATH, "rb");
  if (!file) {
    fprintf(stderr, "Failed to open '%s' file\n", OS_AUDIO_BANK_PATH);
    return 0;
  }

  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);

  if (size < OS_AUDIO_HEADER_SIZE) {
    fprintf(stderr, "'%s' is too small to be a valid sound bank\n", OS_AUDIO_BANK_PATH);
    fclose(file);
    return 0;
  }

  bankData = (unsigned char*) malloc((size_t) size);
  if (!bankData) {
    fprintf(stderr, "Failed to allocate memory for '%s'\n", OS_AUDIO_BANK_PATH);
    fclose(file);
    return 0;
  }

  size_t bytesRead = fread(bankData, 1, (size_t) size, file);
  fclose(file);

  if (bytesRead != (size_t) size || memcmp(bankData, OS_AUDIO_BANK_MAGIC, 4) != 0) {
    fprintf(stderr, "'%s' is not a valid sound bank\n", OS_AUDIO_BANK_PATH);
    free(bankData);
    bankData = NULL;
    return 0;
  }

  bankClipCount = audio_readU32LE(bankData + 8);
  return 1;
}

static int audio_initClipSound(const char *name, ma_audio_buffer_ref *outRef, ma_sound *outSound, double *outDurationSeconds) {
  const unsigned char *entry = audio_findClipEntry(name);
  if (!entry) {
    fprintf(stderr, "Sound bank '%s' is missing clip '%s'\n", OS_AUDIO_BANK_PATH, name);
    return 0;
  }

  ma_format format = (ma_format) audio_readU32LE(entry + OS_AUDIO_CLIP_NAME_SIZE);
  ma_uint32 channels = audio_readU32LE(entry + OS_AUDIO_CLIP_NAME_SIZE + 4);
  ma_uint32 sampleRate = audio_readU32LE(entry + OS_AUDIO_CLIP_NAME_SIZE + 8);
  ma_uint64 frameCount = audio_readU64LE(entry + OS_AUDIO_CLIP_NAME_SIZE + 12);
  ma_uint64 dataOffset = audio_readU64LE(entry + OS_AUDIO_CLIP_NAME_SIZE + 20);

  if (ma_audio_buffer_ref_init(format, channels, bankData + dataOffset, frameCount, outRef) != MA_SUCCESS) {
    fprintf(stderr, "Failed to reference clip '%s'\n", name);
    return 0;
  }
  outRef->sampleRate = sampleRate; /* Not set by ma_audio_buffer_ref_init in this miniaudio version. */

  if (ma_sound_init_from_data_source(&engine, outRef, 0, NULL, outSound) != MA_SUCCESS) {
    fprintf(stderr, "Failed to initialize sound for clip '%s'\n", name);
    ma_audio_buffer_ref_uninit(outRef);
    return 0;
  }

  if (outDurationSeconds) {
    *outDurationSeconds = (double) frameCount / (double) sampleRate;
  }
  return 1;
}

static void audio_generateFireBuffer(ma_uint32 sampleRate) {
  int totalClicks = OS_FIRE_NOISE_CLICK_COUNT + OS_FIRE_TONE_STEP_COUNT;
  double *clickDurations = (double*) malloc(sizeof(double) * totalClicks);
  int clickIndex = 0;
  double totalDurationSeconds = 0.0;

  for (int i = 0; i < OS_FIRE_NOISE_CLICK_COUNT; i++) {
    int delay = 1 + (rand() % 127);
    double seconds = (17.0 + 5.0 * delay) / OS_FIRE_CLOCK_HZ;
    clickDurations[clickIndex++] = seconds;
    totalDurationSeconds += seconds;
  }

  for (int period = 1; period <= OS_FIRE_TONE_STEP_COUNT; period++) {
    double seconds = (5.0 * period + 14.0) / OS_FIRE_CLOCK_HZ;
    clickDurations[clickIndex++] = seconds;
    totalDurationSeconds += seconds;
  }

  fireFrameCount = (ma_uint32)(sampleRate * totalDurationSeconds) + 1;
  firePCM = (float*) malloc(sizeof(float) * fireFrameCount);

  double hardwareTime = 0.0;
  float polarity = (float) OS_FIRE_AMPLITUDE;
  ma_uint32 outputIndex = 0;

  for (int i = 0; i < totalClicks; i++) {
    double targetTime = hardwareTime + clickDurations[i];
    while (outputIndex < fireFrameCount && (double) outputIndex / (double) sampleRate < targetTime) {
      firePCM[outputIndex] = polarity;
      outputIndex++;
    }
    hardwareTime = targetTime;
    polarity = -polarity;
  }

  while (outputIndex < fireFrameCount) {
    firePCM[outputIndex] = 0.0f;
    outputIndex++;
  }

  free(clickDurations);
}

int audio_init() {
  if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
    fprintf(stderr, "Failed to initialize audio engine\n");
    return 0;
  }

  ma_waveform_config waveformConfig = ma_waveform_config_init(
    ma_format_f32,
    ma_engine_get_channels(&engine),
    ma_engine_get_sample_rate(&engine),
    ma_waveform_type_square,
    OS_BELL_AMPLITUDE,
    OS_BELL_FREQUENCY
  );

  if (ma_waveform_init(&waveformConfig, &bellWaveform) != MA_SUCCESS) {
    fprintf(stderr, "Failed to initialize bell waveform\n");
    ma_engine_uninit(&engine);
    return 0;
  }

  if (ma_sound_init_from_data_source(&engine, &bellWaveform, 0, NULL, &bellSound) != MA_SUCCESS) {
    fprintf(stderr, "Failed to initialize bell sound\n");
    ma_waveform_uninit(&bellWaveform);
    ma_engine_uninit(&engine);
    return 0;
  }

  if (!audio_loadBank()) {
    ma_sound_uninit(&bellSound);
    ma_waveform_uninit(&bellWaveform);
    ma_engine_uninit(&engine);
    return 0;
  }

  if (!audio_initClipSound("star-click", &engineClickRef, &engineHumSound, NULL)) {
    free(bankData);
    ma_sound_uninit(&bellSound);
    ma_waveform_uninit(&bellWaveform);
    ma_engine_uninit(&engine);
    return 0;
  }

  if (!audio_initClipSound("alert", &alertClipRef, &alertSound, &alertClipDurationSeconds)) {
    free(bankData);
    ma_sound_uninit(&engineHumSound);
    ma_audio_buffer_ref_uninit(&engineClickRef);
    ma_sound_uninit(&bellSound);
    ma_waveform_uninit(&bellWaveform);
    ma_engine_uninit(&engine);
    return 0;
  }
  ma_sound_set_looping(&alertSound, MA_TRUE);

  audio_generateFireBuffer(ma_engine_get_sample_rate(&engine));

  if (ma_audio_buffer_ref_init(ma_format_f32, 1, firePCM, fireFrameCount, &fireBufferRef) != MA_SUCCESS) {
    fprintf(stderr, "Failed to reference fire buffer\n");
    free(firePCM);
    ma_sound_uninit(&alertSound);
    ma_audio_buffer_ref_uninit(&alertClipRef);
    free(bankData);
    ma_sound_uninit(&engineHumSound);
    ma_audio_buffer_ref_uninit(&engineClickRef);
    ma_sound_uninit(&bellSound);
    ma_waveform_uninit(&bellWaveform);
    ma_engine_uninit(&engine);
    return 0;
  }
  fireBufferRef.sampleRate = ma_engine_get_sample_rate(&engine); /* Not set by ma_audio_buffer_ref_init in this miniaudio version. */

  if (ma_sound_init_from_data_source(&engine, &fireBufferRef, 0, NULL, &fireSound) != MA_SUCCESS) {
    fprintf(stderr, "Failed to initialize fire sound\n");
    ma_audio_buffer_ref_uninit(&fireBufferRef);
    free(firePCM);
    ma_sound_uninit(&alertSound);
    ma_audio_buffer_ref_uninit(&alertClipRef);
    free(bankData);
    ma_sound_uninit(&engineHumSound);
    ma_audio_buffer_ref_uninit(&engineClickRef);
    ma_sound_uninit(&bellSound);
    ma_waveform_uninit(&bellWaveform);
    ma_engine_uninit(&engine);
    return 0;
  }

  initialized = 1;
  return 1;
}

void audio_playBell() {
  if (!initialized) {
    return;
  }

  ma_uint64 startFrame = ma_engine_get_time_in_pcm_frames(&engine);
  ma_uint64 durationInFrames = (ma_uint64)(ma_engine_get_sample_rate(&engine) * OS_BELL_DURATION_SECONDS);

  ma_sound_stop(&bellSound);
  ma_sound_seek_to_pcm_frame(&bellSound, 0);
  ma_sound_set_stop_time_in_pcm_frames(&bellSound, startFrame + durationInFrames);
  ma_sound_start(&bellSound);
}

void audio_playAlert(int repeatCount) {
  if (!initialized) {
    return;
  }

  if (repeatCount < 1) {
    repeatCount = 1;
  }

  ma_uint64 clipFrames = (ma_uint64)(alertClipDurationSeconds * ma_engine_get_sample_rate(&engine));
  ma_uint64 startFrame = ma_engine_get_time_in_pcm_frames(&engine);
  ma_uint64 totalFrames = clipFrames * (ma_uint64) repeatCount;

  ma_sound_stop(&alertSound);
  ma_sound_seek_to_pcm_frame(&alertSound, 0);
  ma_sound_set_stop_time_in_pcm_frames(&alertSound, startFrame + totalFrames);
  ma_sound_start(&alertSound);
}

void audio_playFire() {
  if (!initialized) {
    return;
  }

  ma_sound_stop(&fireSound);
  ma_sound_seek_to_pcm_frame(&fireSound, 0);
  ma_sound_start(&fireSound);
}

void audio_startEngineHum() {
  if (!initialized) {
    return;
  }

  engineHumActive = 1;
  engineHumTimer = 0.0;
}

void audio_setEngineHumSpeed(float speedModifier) {
  if (!initialized) {
    return;
  }

  engineHumSpeedModifier = speedModifier;

  double targetFrequency = OS_ENGINE_CLICK_BASE_FREQUENCY + (speedModifier - 1.0) * OS_ENGINE_CLICK_FREQUENCY_PER_SPEED;
  ma_sound_set_pitch(&engineHumSound, (float)(targetFrequency / OS_ENGINE_CLICK_BASE_FREQUENCY));
}

void audio_updateEngineHum(float deltaTime) {
  if (!initialized || !engineHumActive) {
    return;
  }

  double clicksPerSecond = OS_ENGINE_CLICK_RATE_BASE + (engineHumSpeedModifier - 1.0) * OS_ENGINE_CLICK_RATE_PER_SPEED;
  double interval = 1.0 / clicksPerSecond;

  engineHumTimer -= (double) deltaTime;
  if (engineHumTimer > 0.0) {
    return;
  }
  engineHumTimer = interval;

  ma_sound_stop(&engineHumSound);
  ma_sound_seek_to_pcm_frame(&engineHumSound, 0);
  ma_sound_start(&engineHumSound);
}

void audio_stopEngineHum() {
  if (!initialized) {
    return;
  }

  engineHumActive = 0;
  ma_sound_stop(&engineHumSound);
}

void audio_cleanup() {
  if (!initialized) {
    return;
  }

  ma_sound_uninit(&bellSound);
  ma_waveform_uninit(&bellWaveform);
  ma_sound_uninit(&engineHumSound);
  ma_audio_buffer_ref_uninit(&engineClickRef);
  ma_sound_uninit(&alertSound);
  ma_audio_buffer_ref_uninit(&alertClipRef);
  ma_sound_uninit(&fireSound);
  ma_audio_buffer_ref_uninit(&fireBufferRef);
  free(firePCM);
  firePCM = NULL;
  free(bankData);
  bankData = NULL;
  ma_engine_uninit(&engine);
  initialized = 0;
}
