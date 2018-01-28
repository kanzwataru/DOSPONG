#include "src/snd.h"
#include <dos.h>

static unsigned int _freq = 0;
static unsigned int _duration = 0;
static unsigned int _ticks = 0;
static unsigned int _slide = 0;

void sound_stopall(void)
{
    nosound();
}

void sound_play(unsigned int tone, unsigned int duration)
{
    _freq = tone;
    _duration = duration;
    _ticks = 0;
    _slide = 0;

    nosound();
    sound(tone);
}

void sound_play_sliding(unsigned int tone, unsigned int duration, unsigned int slide)
{
    sound_play(tone, duration);

    _slide = slide;
}

void sound_tick(void)
{
    if(_duration > 0) {
        if(_slide != 0) {
            _freq += _slide;
            sound(_freq);
        }

        if(++_ticks > _duration) {
            _duration = 0;
            _ticks = 0;
            nosound();
        }
    }
}
