#include "src/srender.h"

#include <malloc.h>
#include <dos.h>

#define INPUT_STATUS_0 0x3da
#define ESC 27
/* #define DRAW_PIXEL(buf, x, y, colour) ((buf)[((y) << 8) + ((y) << 6) + (x)] = (colour))
*/

static int old_mode;

static unsigned char far *make_framebuffer() {
	return farmalloc(SCREEN_SIZE);
}

static void draw_pixel(unsigned char far *buf, int x, int y, int colour) {
	buf[(y << 8) + (y << 6) + x] = colour;
}

static int get_pixel(unsigned char far *buf, int x, int y) {
	return *(buf + y * SCREEN_WIDTH + x);
}

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
		rd->screen = MK_FP(0xa000, 0); /* this gets the screen framebuffer */
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
    size_t size = count * sizeof(Rect);

    rd->rect_count = count;
    rd->dirty_rects = malloc(size);
    rd->rects = malloc(size);
}

void free_rects(RenderData *rd)
{
    free(rd->dirty_rects);
    free(rd->rects);
}

void blit_buffer(unsigned char far *dest, const unsigned char far *src, const Rect *rect)
{
	static int x, y, x_max, y_max, offset;

	x_max = rect->x + rect->w;
	y_max = rect->y + rect->h;

	for(x = rect->x; x < x_max; ++x) {
		for(y = rect->y; y < y_max; ++y) {
			offset = (y << 8) + (y << 6) + x;

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
		offset = (y << 8) + (y << 6) + rect->x;
		_fmemcpy(dest + offset, src + offset, rect->w);
	}
}

void fill(unsigned char far *buf, int colour)
{
	int x, y;
	for(x = 0; x < SCREEN_WIDTH; x++) {
		for(y = 0; y < SCREEN_HEIGHT; y++) {
			draw_pixel(buf, x, y, colour);
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
			    || x >= SCREEN_WIDTH -  border - 5
			    || y >= SCREEN_HEIGHT - border - 1)
			{
				draw_pixel(buf, x, y, border_colour);
			}
			else {
				draw_pixel(buf, x, y, colour);
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
			draw_pixel(buf, x, y, colour);
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
		offset = (y << 8) + (y << 6) + rect->x;
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
		draw_rect_fast(rd->screen, &rd->rects[i], i + 1);
	}
}
