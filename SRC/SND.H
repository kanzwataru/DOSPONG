#ifndef SND_H
#define SND_H

#define SILENT     0
#define NOSLIDE    0

void sound_stopall(void);
void sound_add(unsigned int freq, unsigned int duration, unsigned int slide);
void sound_tick(void);

#endif
