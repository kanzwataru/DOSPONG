#include "src/dospong.h"

#include <stdio.h>
#include <conio.h>
#include <malloc.h>
#include <dos.h>

#include "src/common.h"
#include "src/srender.h"
#include "src/pctimer.h" /* One of two places included, but not used at the same time */
#include "src/pcinput.h"
#include "src/utils.h"
#include "src/snd.h"

/* VGA default pallete colours */
#define BLACK 0
#define WHITE 15
#define GRAY  7
#define DGRAY 8
/* */

/* Sounds */
#define GRR_SND        50
#define GRR_LEN         1
#define BOOP_SND      440
#define BOOP_LEN        2
#define PLAYER_SND    640
#define PLAYER_LEN      8
#define AI_SND         80
#define AI_LEN		   15
/* */

/* Shape constants */
#define PADDLE_H   40
#define PADDLE_W   10
#define BALL_W     10
#define WALL_W     10
#define SCORE_W    6
#define SCORE_H    2

static const int PADDLE_HALFH = PADDLE_H >> 1;
static const int PADDLE_HALFW = PADDLE_W >> 1;
/* */

/* Positioning constants */
#define PADDLE_GAP   120
#define SCORE_OFFSET 3
static const int SCR_HALF      = SCREEN_WIDTH / 2;
static const int TOP           = WALL_W;
static const int BTM           = SCREEN_HEIGHT - WALL_W;
static const int BTM_P         = SCREEN_HEIGHT - WALL_W - PADDLE_H;
static const int AI_MIN_REACT  = SCREEN_WIDTH / 2 - 50;
static const int AI_MAX_REACT  = SCREEN_WIDTH / 2 + PADDLE_GAP - 12;
static const int AI_STOP_RANGE = 5;
/* */

/* Speed constants */
#define PADDLE_ACCEL       70
#define PADDLE_DECEL	  120
#define PADDLE_SPEED      600
#define PADDLE_FPMULT     100  /* Fixed-point multiplier */
#define BALL_SPEED        	2

#define AI_PREDICT_FRAMES  19
/* */

/* Data types */
typedef struct {
	Rect *rect;
	int speed;
	int direction;
	int score;
} Paddle;

typedef struct {
	Rect *rect;
	Rect *parent;
	int dir_x;
	int dir_y;
} Ball;
/* */

/* game vars */
static unsigned char bg_col = BLACK;
static unsigned char accent_col = GRAY;
static BOOL game_paused = FALSE;
static Paddle player;
static Paddle ai;
static Ball ball;
/* */

/* graphics */
static RenderData rd;
/* */

static void draw_score_tiles(void)
{
	int ai_score_pos = SCREEN_WIDTH - SCORE_OFFSET;
	Rect rect = {0, SCORE_OFFSET, SCORE_H, SCORE_W};

	int i;
	for(i = 0; i < player.score; ++i) {
		rect.x = SCORE_OFFSET + (i * (SCORE_W));
		draw_rect_fast(rd.bg_layer, &rect, BLACK);
	}

	for(i = 0; i < ai.score; ++i) {
		rect.x = ai_score_pos - (i * (SCORE_W));
		draw_rect_fast(rd.bg_layer, &rect, BLACK);
	}
}

static void update_bg(void)
{
	fill_bordered(rd.bg_layer, bg_col, accent_col, WALL_W);
	draw_score_tiles();

	_fmemcpy(rd.screen, rd.bg_layer, SCREEN_SIZE);
}

static void shuffle_cols(void)
{
	++accent_col;
	if(accent_col == 16)
		accent_col += 18; /* Skip over the monochromatic colours */

	update_bg();

	player.rect->col = accent_col;
	ai.rect->col = accent_col;
	ball.rect->col = accent_col;
}

static void pause(void)
{
	static unsigned char col_a, col_b;

	game_paused = !game_paused;
	if(game_paused) {
		col_a = bg_col;
		col_b = accent_col;

		player.rect->col = GRAY;
		ai.rect->col = GRAY;
		ball.rect->col = GRAY;

		bg_col = DGRAY;
		accent_col = GRAY;

		update_bg();
	}
	else {
		player.rect->col = col_b;
		ai.rect->col = col_b;
		ball.rect->col = col_b;

		bg_col = col_a;
		accent_col = col_b;

		update_bg();
	}
}

static void update_paddle(Paddle *paddle)
{
	paddle->speed = f_clamp((paddle->speed + (PADDLE_ACCEL * paddle->direction))
										   * (1.0f - (0.04f * !abs(paddle->direction))),
							  	  				-PADDLE_SPEED, PADDLE_SPEED);

	paddle->rect->y = paddle->rect->y + (paddle->speed / PADDLE_FPMULT);
	if(paddle->rect->y < TOP) {
		paddle->rect->y = TOP;
		paddle->speed = 0;
	}
	if(paddle->rect->y > BTM_P) {
		paddle->rect->y = BTM_P;
		paddle->speed = 0;
	}
}

static void ball_reset(void)
{
	ball.parent = player.rect;
	ball.dir_x = 1;
	ball.dir_y = 1;
}

static void ball_world_collision(void)
{
	if(ball.rect->y < TOP) {
		ball.dir_y = abs(ball.dir_y);
		sound_play(GRR_SND, GRR_LEN);
	}
	if(ball.rect->y + ball.rect->h > BTM) {
		ball.dir_y = -(abs(ball.dir_y));
		sound_play(GRR_SND, GRR_LEN);
	}

	if(ball.rect->x < 0) { /* LEFT */
		sound_play(AI_SND, AI_LEN);

		++ai.score;
		ball_reset();
		shuffle_cols();
	}
	if(ball.rect->x + ball.rect->w > SCREEN_WIDTH) { /* RIGHT */
		sound_play_sliding(PLAYER_SND, PLAYER_LEN, 50);

		++player.score;
		ball_reset();
		shuffle_cols();
	}
}

static void ball_paddle_collision(const Paddle *paddle)
{
	static const int LEFT = 1;
	static const int RIGHT = -1;

	int percent = remap(ball.rect->y, paddle->rect->y,
									    paddle->rect->y + (paddle->rect->h),
									    -1, 1);

	int side = paddle->rect->x > SCR_HALF ? RIGHT : LEFT;

	if(percent < 1 && percent > -1) {
		if((side == LEFT  && ball.rect->x < (paddle->rect->x + paddle->rect->w) && ball.rect->x + ball.rect->w > paddle->rect->x)
		|| (side == RIGHT && ball.rect->x + ball.rect->w > paddle->rect->x && ball.rect->x < paddle->rect->x + paddle->rect->w))
		{
			ball.dir_y = percent + ((paddle->speed / 100) >> 2) + paddle->direction;
			ball.dir_x = side;

			sound_play(BOOP_SND, BOOP_LEN);
		}
	}
}

static void update_ball(void)
{
	int side;

	if(ball.parent) {
		side = (ball.parent->x > SCR_HALF) ? 0 : 1;  /* don't add the width for the right side paddle */

		ball.rect->x = (ball.parent->x + (side * ball.parent->w)
									   + (!side * ball.rect->w));
		ball.rect->y = ball.parent->y + (ball.parent->h >> 1) - (ball.rect->h >> 1);
	}
	else {
		ball.rect->x += (BALL_SPEED * ball.dir_x);
		ball.rect->y += (BALL_SPEED * ball.dir_y);
	}
}

static int ai_predict(void)
{
	static int predict_counter;
	static int last_prediction;

	if(predict_counter > AI_PREDICT_FRAMES) {
		predict_counter = 0;
		last_prediction = ball.rect->y + (ball.rect->h >> 1);
	}
	else {
		++predict_counter;
	}

	return last_prediction;
}

static void update_ai(void)
{
	int prediction = ai_predict();
	//int prediction = ball.rect->y + (ball.rect->h >> 1);
	int mid = ai.rect->y + PADDLE_HALFH;

	if(ball.rect->x > AI_MIN_REACT && ball.rect->x < AI_MAX_REACT) {
		ai.direction = (prediction < mid) ? -1 : ai.direction;
		ai.direction = (prediction > mid) ?  1 : ai.direction;
	}
	else {
		ai.direction = 0;
	}

	if(prediction > (mid - AI_STOP_RANGE)
	&& prediction < (mid + AI_STOP_RANGE))
	{
		ai.direction = 0;
	}

	if(ball.dir_x < 0)
		ai.direction = 0;
}

static void update(void)
{
	if(game_paused)
		return;

	update_paddle(&player);
	update_paddle(&ai);

	ball_world_collision();
	ball_paddle_collision(&player);
	ball_paddle_collision(&ai);
	update_ball();
	update_ai();
}

static BOOL handle_input(void)
{
	switch(read_scancode()) {
		case ESC:
			return FALSE;
			break;
		case 27: /* ']' (SECRET) */
			shuffle_cols();
			break;
		case SPACE:
			ball.parent = NULL;
			break;
		case UP_ARROW:
			player.direction = -1;
			break;
		case DOWN_ARROW:
			player.direction = 1;
			break;
		case PAUSE:
			pause();
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

	free_keyb_buf();

	return TRUE;
}

static void game_loop(void)
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

		sound_tick();
		render_fast(&rd);
	}
}

static void quit(void)
{
	quit_renderer(&rd);
	free_rects(&rd);
	restore_timer();
	sound_stopall();

	exit(1);
}

int pong_init(void)
{
	int i;
	init_renderer(&rd);
	init_rects(&rd, 3); /* Three rects: ball, player, ai */
	init_timer();

	printf("*** DOSPONG ***\n");
	printf("%s\n", VERSION);
	printf("Still DOS-ing in 2018\n");
	printf("\n Loading... \n");

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
	ball.parent = player.rect;
	ball.dir_y = 1;
	ball.dir_x = 1;

	update_bg();

	game_loop();

	quit();
    return 0;
}
