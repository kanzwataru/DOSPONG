#include "src/dospong.h"

#include <stdio.h>
#include <conio.h>
#include <malloc.h>
#include <dos.h>

#include "src/srender.h"
#include "src/pctimer.h" /* One of two places included, but not used at the same time */
#include "src/pcinput.h"
#include "src/utils.h"

typedef int BOOL;
#define TRUE  1
#define FALSE 0

/* keyboard keys */
#define UP_ARROW   72
#define DOWN_ARROW 80
#define PAUSE      25
#define SPACE      57
#define ESC        1
/* */

/* VGA default pallete colours */
#define BLACK 0
#define WHITE 15
#define GRAY  7
#define DGRAY 8
/* */

/* Shape constants */
#define PADDLE_H   30
#define PADDLE_W   10
#define BALL_W     10
#define WALL_W     10
static const int PADDLE_HALFH = PADDLE_H >> 1;
static const int PADDLE_HALFW = PADDLE_W >> 1;
/* */

/* Positioning constants */
#define PADDLE_GAP 120
static const int SCR_HALF = SCREEN_WIDTH / 2;
static const int TOP      = WALL_W;
static const int BTM      = SCREEN_HEIGHT - WALL_W - PADDLE_H;
/* */

/* Speed constants */
#define PADDLE_ACCEL       60
#define PADDLE_SPEED      600
#define BALL_SPEED        500

#define P_ACCEL_FRAMES    6
#define AI_PREDICT_FRAMES 12
/* */

/* Data types */
typedef struct {
	Rect *rect;
	int speed;
	int direction;
} Paddle;

typedef struct {
	unsigned char frame_count;
	unsigned char y_predict;
} AIAttrs;

typedef struct {
	Rect *rect;
	Rect *parent;
	int dir_x;
	int dir_y;
} Ball;
/* */

/* game vars */
unsigned char bg_col = BLACK;
unsigned char accent_col = GRAY;
BOOL game_paused = FALSE;
Paddle player;
Paddle ai;
AIAttrs aiattrs;
Ball ball;
/* */
static RenderData rd;

void update_bg(void)
{
	fill_bordered(rd.bg_layer, bg_col, accent_col, WALL_W);
	_fmemcpy(rd.screen, rd.bg_layer, SCREEN_SIZE);
}

void shuffle_cols(void)
{
	++accent_col;

	update_bg();

	player.rect->col = accent_col;
	ai.rect->col = accent_col;
	ball.rect->col = accent_col;
}

void update_paddle(Paddle *paddle)
{
	paddle->speed = f_clamp((paddle->speed + (PADDLE_ACCEL * paddle->direction))
										   * (1.0f - (0.01f * !abs(paddle->direction))),
							  	  				-PADDLE_SPEED, PADDLE_SPEED);

	paddle->rect->y = paddle->rect->y + (paddle->speed / 100);
	if(paddle->rect->y < TOP) {
		paddle->rect->y = TOP;
		paddle->speed = 0;
	}
	if(paddle->rect->y > BTM) {
		paddle->rect->y = BTM;
		paddle->speed = 0;
	}
}

/*
void update_paddle(Paddle *paddle)
{
	paddle->rect->y += paddle->direction;
}
*/

void update_ball(void)
{

}

void update_ai(void)
{

}

void update(void)
{
	update_paddle(&player);
	update_paddle(&ai);

	update_ball();
	update_ai();
}

BOOL handle_input(void)
{
	switch(read_scancode()) {
		case ESC:
			return FALSE;
			break;
		case 27: /* ']' (SECRET) */
			shuffle_cols();
			break;
		case UP_ARROW:
			player.direction = -1;
			break;
		case DOWN_ARROW:
			player.direction = 1;
			break;
		case RELEASED(UP_ARROW):
			player.direction = 0;
			break;
		case RELEASED(DOWN_ARROW):
			player.direction = 0;
			break;
		default:
			return TRUE;
			break;
	}

	return TRUE;
}

void game_loop(void)
{
	int i;
	unsigned long last_time = 0l;
	unsigned long accumulator = 0l;
	unsigned long current_time;
	unsigned long delta_time;

	while(handle_input()) {
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

		render_fast(&rd);
	}
}

void quit(void)
{
	quit_renderer(&rd);
	free_rects(&rd);
	restore_timer();
	exit(1);
}

int pong_init(void)
{
	int i;
	init_renderer(&rd);
	init_rects(&rd, 3); /* Three rects: ball, player, ai */
	init_timer();

	player.rect = &rd.rects[0];
	player.rect->x = SCR_HALF - PADDLE_GAP;
	player.rect->y = SCREEN_HEIGHT / 2 - PADDLE_HALFH;
	player.rect->w = PADDLE_W;
	player.rect->h = PADDLE_H;
	player.rect->col = accent_col;

	ai.rect    = &rd.rects[1];
	ai.rect->x = SCR_HALF + PADDLE_GAP - PADDLE_W;
	ai.rect->y = SCREEN_HEIGHT / 2 - PADDLE_HALFH;
	ai.rect->w = PADDLE_W;
	ai.rect->h = PADDLE_H;
	ai.rect->col = accent_col;

	ball.rect   = &rd.rects[2];
	ball.rect->w = BALL_W;
	ball.rect->h = BALL_W;
	ball.rect->col = accent_col;

	update_bg();

	game_loop();

	quit();
    return 0;
}
