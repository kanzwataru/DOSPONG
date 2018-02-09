/*
 * Nothing to see here, this menu code is not very good
*/
#include "src/pongmenu.h"

#include <conio.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "src/common.h"
#include "src/dospong.h"

#define FOREGROUND_COL  8
#define BACKGROUND_COL  7
#define LABEL_COL      18

enum view {
    PLAYER_SELECT_VIEW,
    DIFFICULTY_SELECT_VIEW
};

static RenderData rd;
static unsigned char far *playersel_labels;
static unsigned char far *difficultysel_labels;

static int current_view = PLAYER_SELECT_VIEW;
static int current_button = 0;
static BOOL start_game = FALSE;

static void load_image(const char *filepath, unsigned char far *b)
{
    FILE *fp;

    if(NULL == (fp = fopen(filepath,"rb"))) {
        printf("Error loading %s\n", filepath);
        exit(1);
    }

    fread(b, sizeof(unsigned char), SCREEN_SIZE, fp);
    fclose(fp);
}

static void load_menu_bg(void)
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

static void load_label_graphics(void)
{
    playersel_labels = farmalloc(SCREEN_SIZE);
    difficultysel_labels = farmalloc(SCREEN_SIZE);

    load_image("GRAPHICS\\PLSEL.DAT", playersel_labels);
    load_image("GRAPHICS\\DIFSEL.DAT", difficultysel_labels);
}

static void draw_text_labels(unsigned char far *image)
{
    unsigned int i;

    for(i = 0; i < SCREEN_SIZE; ++i) {
        if(image[i] == 1)
            rd.back_buf[i] = LABEL_COL;
    }
}

static void move(int dir)
{
    current_button -= dir;
    if(current_view == PLAYER_SELECT_VIEW && current_button > 1)
        current_button = 0;
    if(current_view == DIFFICULTY_SELECT_VIEW && current_button > 2)
        current_button = 0;

    if(current_view == PLAYER_SELECT_VIEW && current_button < 0)
        current_button = 1;
    if(current_view == DIFFICULTY_SELECT_VIEW && current_button < 0)
        current_button = 2;
}

static void select(void)
{
    switch(current_view) {
        case PLAYER_SELECT_VIEW:
            if(current_button == 0) {
                current_view = DIFFICULTY_SELECT_VIEW;
                current_button = 1;
            }
            break;

        case DIFFICULTY_SELECT_VIEW:
            start_game = TRUE;
    }
}

static BOOL handle_input(void)
{
    switch(getch()) {
        case 27: /* ESC */
            if(current_view == PLAYER_SELECT_VIEW)
                return FALSE;
            else {
                current_view = PLAYER_SELECT_VIEW;
                current_button = 0;
            }
            break;
        case UP_ARROW:
            move(1);
            break;
        case DOWN_ARROW:
            move(-1);
            break;
        case 13: /* ENTER */
            select();
            break;
        default:
            return TRUE;
    }

    return TRUE;
}

int pong_menu_init(void)
{
    Rect sel_rect = {SCREEN_WIDTH / 2 - (190 / 2 - 2), 118, 190, 18, 3};

    init_renderer(&rd);

    load_menu_bg();
    load_label_graphics();

    do {
        _fmemcpy(rd.back_buf, rd.bg_layer, SCREEN_SIZE);

        sel_rect.y = 118 + (current_button * 23);
        draw_rect_fast(rd.back_buf, &sel_rect, FOREGROUND_COL);

        if(current_view == PLAYER_SELECT_VIEW)
            draw_text_labels(playersel_labels);
        else
            draw_text_labels(difficultysel_labels);

        flip_buffer_full(&rd);

        if(start_game)
            break;
    } while(handle_input());

    if(start_game)
        pong_init(&rd);
    else
        quit_renderer(&rd);

    farfree(playersel_labels);
    farfree(difficultysel_labels);

    return 0;
}
