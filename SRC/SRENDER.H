#ifndef SRENDER_H
#define SRENDER_H

#define UPDATE_STEP_SIZE 2
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define SCREEN_SIZE 64000u

typedef struct {
	int x;
	int y;
	int w;
	int h;
} Rect;

typedef struct {
    int rect_count;
    Rect *dirty_rects;
    Rect *rects;
    unsigned char far *screen;
    unsigned char far *back_buf;
    unsigned char far *bg_layer;
} RenderData;

int init_renderer(RenderData *rd);
void quit_renderer(RenderData *rd);

void init_rects(RenderData *rd, int count);
void free_rects(RenderData *rd);

void blit_buffer(unsigned char far *dest, const unsigned char far *src, const Rect *rect);
void blit_buffer_fast(unsigned char far *dest, const unsigned char far *src, const Rect *rect);
void fill(unsigned char far *buf, int colour);
void fill_fast(unsigned char far *buf, int colour);
void fill_bordered(unsigned char far *buf, int colour, int border_colour, int border);
void draw_rect(unsigned char far *buf, const Rect *rect, int colour);
void draw_rect_fast(unsigned char far *buf, const Rect *rect, int colour);
void clear(RenderData *rd);
void flip_buffer(RenderData *rd);
void render_fast(RenderData *rd);

#endif /* SRENDER_H */
