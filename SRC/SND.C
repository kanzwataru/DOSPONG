#include "src/snd.h"
#include <dos.h>

#define MAX_SOUNDS 6

typedef struct {
    unsigned int freq;
    unsigned int duration;
    unsigned int ticks;
    unsigned int slide;
} Sound;

static Sound stack[MAX_SOUNDS];
static int current = 0;
static int top = 0;

static void next_sound()
{
    if(++current > top) {
        current = 0;
        top = 0;
    }

    if(stack[current].freq != SILENT)
        sound(stack[current].freq);
}

void sound_add(unsigned int freq, unsigned int duration, unsigned int slide)
{
    if(top + 1 < MAX_SOUNDS) {
        ++top;

        stack[top].freq = freq;
        stack[top].duration = duration;
        stack[top].slide = slide;
    }
}

void sound_tick(void)
{
    if(stack[current].duration == 0)
        next_sound();

    if(++stack[current].ticks > stack[current].duration) {
        stack[current].freq = SILENT;
        stack[current].duration = 0;
        stack[current].ticks = 0;
        stack[current].slide = 0;
        nosound();

        next_sound();
    }
}

void sound_stopall(void)
{
    current = 0;
    top = 0;

    nosound();
}
