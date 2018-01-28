/*
 * Square RENDERER -> SRENDER
 * (because Rectangle RENDERER -> RRENDER sounds silly)
 *
 * Just a simple, decently fast renderer that draws only rectangles
 *
 * There are two main ways of using it, either with double-buffering and
 * individual render commands (clear() -> draw_rect() -> flip_buffer()),
 * or by using render_fast() to render all rectangles
 * in one step directly to the screen memory.
 *
 * With the exception of render_fast, all functions with the _fast suffix do the same
 * thing as their normal counterparts, just with slightly more optimization. There
 * isn't a real reason to use the non-"_fast" versions.
 *
 * It's possible to perform operations directly on the buffers inside RenderData
 * without using these functions.
*/

#ifndef SRENDER_H
#define SRENDER_H

#define SCREEN_WIDTH 320   /* Mode 13h uses 320 x 200 res */
#define SCREEN_HEIGHT 200
#define SCREEN_SIZE 64000u /* Amount of pixels (320 x 200) */

/*
 * Rectangle with colour pallete ID
*/
typedef struct {
	int x;
	int y;
	int w;
	int h;
	unsigned char col;
} Rect;

/*
 * Structure which contains all the data needed to render a frame with
 * the SRENDER functions.
*/
typedef struct {
    int rect_count;               /* Current amount of rectangles */
    Rect *dirty_rects;            /* Previous frame rectangles that need to be painted over with the BG */
    Rect *rects;                  /* Current frame rectangles that need to be filled in */
    unsigned char far *screen;    /* Pointer to screen memory */
    unsigned char far *back_buf;  /* Back buffer that can be swapped to screen if using double-buffering */
    unsigned char far *bg_layer;  /* Background layer that is used to paint over dirty_rects */
} RenderData;

/*
 * init: Initializes the RenderData struct passed in and enters VGA mode 13h
 *       (the rects are not initialized by this function)
 *
 * quit: Frees RenderData struct and leaves VGA mode 13h
*/
int init_renderer(RenderData *rd);
void quit_renderer(RenderData *rd);

/*
 * Initialize and free the rects, zeroes out to avoid bugs from pre-initialized values
*/
void init_rects(RenderData *rd, int count);
void free_rects(RenderData *rd);

/*
 * Clear the screen by drawing over the RenderData::dirty_rects
 * Draws the background layer onto the back buffer
 * Use in conjuction with flip_buffer() and drawing functions
 *
 * (not optimized to detect whether or not the rects moved)
*/
void clear(RenderData *rd);

/*
 * Copies the back buffer to the screen memory, with Vsync
*/
void flip_buffer(RenderData *rd);

/*
 * Combines clearing dirty rects and filling rects in one step,
 * without using a back buffer (draws directly to the screen after vblank)
 *
 * This runs significantly faster (60FPS on DOSBOX's 3000 cycle speed)
 * however it's got ugly artifacts if running at slower than 60FPS
*/
void render_fast(RenderData *rd);

/* ---------------------- Drawing Functions ------------------------ */

/*
 * Plots a pixel in a particular coord
 *
 * (not particularly fast)
*/
void draw_pixel(unsigned char far *buf, int x, int y, int colour);

/*
 * Gets the colour of a pixel in a particular coord
 *
 * (not particularly fast)
*/
unsigned char get_pixel(unsigned char far *buf, int x, int y);

/*
 * Copies a buffer into another
 *
 * dest: Destination buffer
 * src: Source buffer
 * rect: The area that should be copied (this is mandatory)
*/
void blit_buffer(unsigned char far *dest, const unsigned char far *src, const Rect *rect);

/*
 * Same as blit_buffer but using memcpy for extra speed
*/
void blit_buffer_fast(unsigned char far *dest, const unsigned char far *src, const Rect *rect);

/*
 * Fill the buffer with a solid colour
*/
void fill(unsigned char far *buf, int colour);

/*
 * Fill the buffer with a solid colour, but faster
*/
void fill_fast(unsigned char far *buf, int colour);

/*
 * Fill the buffer with a border, Commodore 64-style
 * (this function can be slow)
*/
void fill_bordered(unsigned char far *buf, int colour, int border_colour, int border);

/*
 * Draw a filled rectangle with a specific colour
*/
void draw_rect(unsigned char far *buf, const Rect *rect, int colour);

/*
 * Draw a filled rectangle with a specific colour, but faster
*/
void draw_rect_fast(unsigned char far *buf, const Rect *rect, int colour);

#endif /* SRENDER_H */
