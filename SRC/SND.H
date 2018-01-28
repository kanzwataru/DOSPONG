#ifndef SND_H
#define SND_H

void sound_stopall(void);
void sound_play(unsigned int tone, unsigned int duration);
void sound_play_sliding(unsigned int tone, unsigned int duration, unsigned int slide);
void sound_tick(void);

#endif
