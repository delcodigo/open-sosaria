#ifndef OS_LIGHTNING_BOLT_EFFECT_H
#define OS_LIGHTNING_BOLT_EFFECT_H

void lightningBoltEffect_init();
void lightningBoltEffect_cast();
bool lightningBoltEffect_isBusy();
void lightningBoltEffect_update(float deltaTime);
void lightningBoltEffect_render(float *viewMatrix);
void lightningBoltEffect_free();

#endif