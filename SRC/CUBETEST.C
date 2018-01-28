#include "src/cubetest.h"

#include <stdio.h>
#include <conio.h>
#include <malloc.h>
#include <dos.h>

#include "src/srender.h"
#include "src/pctimer.h" /* One of two places included, but not used at the same time */
#include "src/utils.h"

#define UPDATE_STEP_SIZE 2
#define RECT_COUNT 5

static int rect_x_dirs[RECT_COUNT];
static int rect_y_dirs[RECT_COUNT];
static RenderData rd;

/*
int i_range_rand(int min, int max) {
    return rand()%(max-min)+min;
}
*/

static void update(void)
{
	int i;

	for(i = 0; i < RECT_COUNT; ++i) {
		rd.rects[i].x += rect_x_dirs[i];
		rd.rects[i].y += rect_y_dirs[i];

		if(rd.rects[i].x + rd.rects[i].w > SCREEN_WIDTH)
			rect_x_dirs[i] = -1;
		else if(rd.rects[i].x < 0)
			rect_x_dirs[i] = 1;

		if(rd.rects[i].y + rd.rects[i].h > SCREEN_HEIGHT)
			rect_y_dirs[i] = -1;
		else if(rd.rects[i].y < 0)
			rect_y_dirs[i] = 1;
	}
}

static void render(void)
{
	/*
	int i;

	clear(&rd);

	for(i = 0; i < RECT_COUNT; ++i) {
		draw_rect(rd.back_buf, &rd.rects[i], i + 1);
	}

	flip_buffer(&rd);
	*/

	render_fast(&rd);
}

static void game_loop(void)
{
	int i;
	unsigned long last_time = 0l;
	unsigned long accumulator = 0l;
	unsigned long current_time;
	unsigned long delta_time;

	while(!kbhit()) {
		current_time = fast_tick;
		delta_time = current_time - last_time;
		last_time = current_time;

		for(i = 0; i < rd.rect_count; ++i) {
			rd.dirty_rects[i] = rd.rects[i];
		}

		accumulator += delta_time;
		while(accumulator > UPDATE_STEP_SIZE) {
			update();
			accumulator -= UPDATE_STEP_SIZE;
		}

		render();
	}
}

static void quit(void)
{
	quit_renderer(&rd);
	free_rects(&rd);
	restore_timer();
	exit(1);
}

int cubetest_init(void)
{
    int i;
    Rect a = {0, 0, 20, 20};

    init_renderer(&rd);
    init_rects(&rd, RECT_COUNT);
    init_timer();

    for(i = 0; i < rd.rect_count; ++i) {
        //rd.rects[i] = a;
        rd.rects[i] = a;
        rd.rects[i].x = (i * (2 * rd.rects[i].w));
        rd.rects[i].y = (i * (2 * rd.rects[i].h));
		rd.rects[i].col = i + 1;
        //rd.rects[i].x = i_range_rand(0, SCREEN_WIDTH);
        //rd.rects[i].y = i_range_rand(0, SCREEN_HEIGHT);

        rect_x_dirs[i] = 1;
        rect_y_dirs[i] = 1;
    }

    fill_bordered(rd.bg_layer, 0, 2, 10);
    //_fmemcpy(back_buf, bg_layer, SCREEN_SIZE);
    _fmemcpy(rd.screen, rd.bg_layer, SCREEN_SIZE);
    game_loop();

    quit();
    return 0;
}
