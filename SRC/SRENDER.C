/*
 * Square RENDERER -> SRENDER
 * (because Rectangle RENDERER -> RRENDER sounds silly)
 *
 * Most documentation/comments are in SRENDER.H
*/
#include "src/srender.h"
#include <malloc.h>
#include <dos.h>

#define INPUT_STATUS_0 0x3da  /* Used for querying Vblank */

/*
 * Calculates the offset into the buffer (y * SCREEN_WIDTH + x)
 * Using bit shifting instead of mults for a bit of optimization
 *
 * (assumes mode 13h 320x200)
*/
#define CALC_OFFSET(x, y)             (((y) << 8) + ((y) << 6) + (x))

/*
 * Macro versions of draw_pixel() and get_pixel(),
 * since Borland C doesn't support inline functions
*/
#define DRAW_PIXEL(buf, x, y, colour) ((buf)[CALC_OFFSET(x, y)] = (colour))
#define GET_PIXEL(buf, x, y)          ((buf)[CALC_OFFSET(x, y)])

static int old_mode;  /* VGA mode we were in before switching to 13h */

/*
 * Creates a buffer the size of the screen
*/
static unsigned char far *make_framebuffer() {
	return farmalloc(SCREEN_SIZE);
}

/*
 * Enters mode 13h
 *
 * (this code was lifted from: http://www3.telus.net/alexander_russell/course/chapter_1.htm)
*/
static void enter_m13h(void)
{
	union REGS in, out;

	// get old video mode
	in.h.ah = 0xf;
	int86(0x10, &in, &out);
	old_mode = out.h.al;

	// enter mode 13h
	in.h.ah = 0;
	in.h.al = 0x13;
	int86(0x10, &in, &out);
}

/*
 * Exits mode 13h
 *
 * (this code was lifted from: http://www3.telus.net/alexander_russell/course/chapter_1.htm)
*/
static void leave_m13h(void)
{
	union REGS in, out;

	in.h.ah = 0;
	in.h.al = old_mode;
	int86(0x10, &in, &out);
}

int init_renderer(RenderData *rd)
{
	rd->bg_layer = make_framebuffer();
	rd->back_buf = make_framebuffer();

	if(rd->back_buf && rd->bg_layer) {
		rd->screen = MK_FP(0xa000, 0);         /* this gets the screen framebuffer */
		enter_m13h();
		_fmemset(rd->back_buf, 0, SCREEN_SIZE);
		_fmemset(rd->bg_layer, 0, SCREEN_SIZE);
		return 1;
	}
	else {
		leave_m13h();
		printf("out of mem\n");
		return 0;
	}
}

void quit_renderer(RenderData *rd)
{
	farfree(rd->bg_layer);
	farfree(rd->back_buf);
	leave_m13h();
}

void init_rects(RenderData *rd, int count)
{
    rd->rect_count = count;
    rd->dirty_rects = calloc(count, sizeof(Rect));
    rd->rects = calloc(count, sizeof(Rect));
}

void free_rects(RenderData *rd)
{
    free(rd->dirty_rects);
    free(rd->rects);
}

void draw_pixel(unsigned char far *buf, int x, int y, int colour) {
	buf[(y << 8) + (y << 6) + x] = colour;
}

unsigned char get_pixel(unsigned char far *buf, int x, int y) {
	return buf[(y << 8) + (y << 6) + x];
}

void blit_buffer(unsigned char far *dest, const unsigned char far *src, const Rect *rect)
{
	static int x, y, x_max, y_max, offset;

	x_max = rect->x + rect->w;
	y_max = rect->y + rect->h;

	for(x = rect->x; x < x_max; ++x) {
		for(y = rect->y; y < y_max; ++y) {
			//offset = (y << 8) + (y << 6) + x;
			offset = CALC_OFFSET(x, y);

			dest[offset] = src[offset];
		}
	}
}

void blit_buffer_fast(unsigned char far *dest, const unsigned char far *src, const Rect *rect)
{
    /*
    printf("bbf {%d %d %d %d}\n", rect->x, rect->y, rect->w, rect->h);
    */
	int y, offset;
	const int y_max = rect->y + rect->h;

	for(y = rect->y; y < y_max; ++y) {
		//offset = (y << 8) + (y << 6) + rect->x;
		offset = CALC_OFFSET(rect->x, y);
		_fmemcpy(dest + offset, src + offset, rect->w);
	}
}

void fill(unsigned char far *buf, int colour)
{
	int x, y;
	for(x = 0; x < SCREEN_WIDTH; x++) {
		for(y = 0; y < SCREEN_HEIGHT; y++) {
			DRAW_PIXEL(buf, x, y, colour);
		}
	}
}

void fill_fast(unsigned char far *buf, int colour)
{
	_fmemset(buf, SCREEN_SIZE, colour);
}

void fill_bordered(unsigned char far *buf, int colour, int border_colour, int border)
{
	int x, y;
	for(x = 0; x < SCREEN_WIDTH; x++) {
		for(y = 0; y < SCREEN_HEIGHT; y++)
		{
			if(x <= border || y <= border
			    || x >= SCREEN_WIDTH -  border - 1
			    || y >= SCREEN_HEIGHT - border)
			{
				DRAW_PIXEL(buf, x, y, border_colour);
			}
			else {
				DRAW_PIXEL(buf, x, y, colour);
			}
		}
	}
}

void draw_rect(unsigned char far *buf, const Rect *rect, int colour)
{
	static int x, y;
	const int max_x = rect->x + rect->w;
	const int max_y = rect->y + rect->h;

	for(x = rect->x; x < max_x; ++x) {
		for(y = rect->y; y < max_y; ++y) {
			DRAW_PIXEL(buf, x, y, colour);
		}
	}
}

void draw_rect_fast(unsigned char far *buf, const Rect *rect, int colour)
{
    /*printf("drf {%d %d %d %d} col: %d\n", rect->x, rect->y, rect->w, rect->h, colour);
    */
    int y, offset;
	const int y_max = rect->y + rect->h;

	for(y = rect->y; y < y_max; ++y) {
		//offset = (y << 8) + (y << 6) + rect->x;
		offset = CALC_OFFSET(rect->x, y);
		_fmemset(buf + offset, colour, rect->w);
	}
}

void clear(RenderData *rd) {
	int i;

	for(i = 0; i < rd->rect_count; ++i) {
		blit_buffer_fast(rd->back_buf, rd->bg_layer, &rd->dirty_rects[i]);
	}
}

void flip_buffer(RenderData *rd)
{
	int i;

	//wait for v retrace
	while(inportb(INPUT_STATUS_0) & 8) {;}
	while(!(inportb(INPUT_STATUS_0) & 8)) {;}

	for(i = 0; i < rd->rect_count; ++i) {
		blit_buffer_fast(rd->screen, rd->bg_layer, &rd->dirty_rects[i]);
		blit_buffer_fast(rd->screen, rd->back_buf, &rd->rects[i]);
	}

	//_fmemcpy(rd->screen, rd->back_buf, SCREEN_SIZE);
}

void render_fast(RenderData *rd)
{
	int i;

	//wait for v retrace
	while(inportb(INPUT_STATUS_0) & 8) {;}
	while(!(inportb(INPUT_STATUS_0) & 8)) {;}

	for(i = 0; i < rd->rect_count; ++i) {
		blit_buffer_fast(rd->screen, rd->bg_layer, &rd->dirty_rects[i]);
		draw_rect_fast(rd->screen, &rd->rects[i], rd->rects[i].col);
	}
}
