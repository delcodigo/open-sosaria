#ifndef OS_AUDIO_HEADER
#define OS_AUDIO_HEADER

int audio_init();
void audio_playBell();
void audio_playAlert(int repeatCount);
void audio_cleanup();

#endif
