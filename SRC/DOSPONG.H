#ifndef DOSPONG_H
#define DOSPONG_H

#include "src/srender.h"

enum difficulty {
    DIFFICULTY_EASY    = 0,
    DIFFICULTY_MEDIUM  = 1,
    DIFFICULTY_HARD    = 2
};

void pong_init(const RenderData *render_data, int difficulty);

#endif
