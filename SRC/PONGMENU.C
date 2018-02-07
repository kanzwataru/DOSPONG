#include "src/pongmenu.h"

#include <conio.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "src/common.h"
#include "src/pcinput.h"
#include "src/srender.h"

#define FOREGROUND_COL  8
#define BACKGROUND_COL  7
#define LABEL_COL      18
#define PLAYER_SELECT_VIEW 0
#define DIFFICULTY_SELECT_VIEW 1

RenderData rd;
unsigned char far *playersel_labels;
unsigned char far *difficultysel_labels;

int current_view = PLAYER_SELECT_VIEW;

/*
void load_menu_bg(void)
{
    unsigned int i;

    for(i = 0; i < SCREEN_SIZE; ++i) {
        if(title_graphics_data[i] != 255)
            rd.bg_layer[i] = title_graphics_data[i];
    }

    _fmemcpy(rd.back_buf, rd.bg_layer, SCREEN_SIZE);
}

*/

void load_image(const char *filepath, unsigned char far *b)
{
    FILE *fp;

    if(NULL == (fp = fopen(filepath,"rb"))) {
        printf("Error loading %s\n", filepath);
        exit(1);
    }

    fread(b, sizeof(unsigned char), SCREEN_SIZE, fp);
    fclose(fp);
}

void load_menu_bg(void)
{
    unsigned char far *image;
    unsigned int i;

    image = farmalloc(SCREEN_SIZE);
    load_image("GRAPHICS\\TITLE.DAT", image);

    for(i = 0; i < SCREEN_SIZE; ++i) {
        if(image[i] == 1)
            rd.bg_layer[i] = FOREGROUND_COL;
        else if(image[i] == 0)
            rd.bg_layer[i] = BACKGROUND_COL;
    }

    _fmemcpy(rd.back_buf, rd.bg_layer, SCREEN_SIZE);
    _fmemcpy(rd.screen, rd.back_buf, SCREEN_SIZE);

    farfree(image);
}

void load_label_graphics(void)
{
    playersel_labels = farmalloc(SCREEN_SIZE);
    difficultysel_labels = farmalloc(SCREEN_SIZE);

    load_image("GRAPHICS\\PLSEL.DAT", playersel_labels);
    load_image("GRAPHICS\\DIFSEL.DAT", difficultysel_labels);
}

void draw_text_labels(unsigned char far *image)
{
    unsigned int i;

    for(i = 0; i < SCREEN_SIZE; ++i) {
        if(image[i] == 1)
            rd.back_buf[i] = LABEL_COL;
    }
}

int pong_menu_init(void)
{
    Rect sel_rect = {SCREEN_WIDTH / 2 - (120 / 2 - 2), 118, 120, 18, 3};

    init_renderer(&rd);

    load_menu_bg();
    load_label_graphics();

    while(!kbhit()) {
        _fmemcpy(rd.back_buf, rd.bg_layer, SCREEN_SIZE);

        draw_rect_fast(rd.back_buf, &sel_rect, FOREGROUND_COL);
        draw_text_labels(playersel_labels);

        flip_buffer_full(&rd);
    }

    farfree(playersel_labels);
    farfree(difficultysel_labels);

    quit_renderer(&rd);
    free_rects(&rd);

    exit(1);

    return 0;
}