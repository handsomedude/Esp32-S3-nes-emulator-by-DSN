#ifndef _VID_DRV_H_
#define _VID_DRV_H_

#include "bitmap.h"

typedef struct viddriver_s
{
   /* name of driver */
   const char *name;
   /* init function - return 0 on success, nonzero on failure */
   int (*init)(int width, int height);
   /* clean up after driver (can be NULL) */
   void (*shutdown)(void);
   /* set a video mode - return 0 on success, nonzero on failure */
   int (*set_mode)(int width, int height);
   /* set up a palette */
   void (*set_palette)(rgb_t *palette);
   /* custom bitmap clear (can be NULL) */
   void (*clear)(uint8 color);
   /* lock surface for writing (required) */
   bitmap_t *(*lock_write)(void);
   /* free a locked surface (can be NULL) */
   void (*free_write)(int num_dirties, rect_t *dirty_rects);
   /* custom blitter - num_dirties == -1 if full blit required */
   void (*custom_blit)(bitmap_t *primary, int num_dirties,
                       rect_t *dirty_rects);
   /* immediately invalidate the buffer, i.e. full redraw */
   bool invalidate;
} viddriver_t;

/* TODO: filth */
extern bitmap_t *vid_getbuffer(void);

extern int vid_init(int width, int height, viddriver_t *osd_driver);
extern void vid_shutdown(void);

extern int vid_setmode(int width, int height);
extern void vid_setpalette(rgb_t *pal);

extern void vid_blit(bitmap_t *bitmap, int src_x, int src_y, int dest_x,
                     int dest_y, int blit_width, int blit_height);
extern void vid_flush(void);

#endif