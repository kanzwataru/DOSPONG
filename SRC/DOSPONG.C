#include "src/dospong.h"

#include <stdio.h>
#include <conio.h>
#include <malloc.h>
#include <dos.h>

#include "src/srender.h"
#include "src/pctimer.h"

static RenderData rd;

static void update(void)
{
    
}

static void render(void)
{

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

int pong_init(void)
{
    return 0;
}
