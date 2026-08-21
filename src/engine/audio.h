#ifndef OS_AUDIO_HEADER
#define OS_AUDIO_HEADER

int audio_init();
void audio_playBell();
void audio_playAlert(int repeatCount);
void audio_startEngineHum();
void audio_setEngineHumSpeed(float speedModifier);
void audio_updateEngineHum(float deltaTime);
void audio_stopEngineHum();
void audio_cleanup();

#endif
