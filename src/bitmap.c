#include <stdio.h>
#include <string.h>

#include "noftypes.h"
#include "bitmap.h"

void bmp_clear(const bitmap_t *bitmap, uint8 color)
{
   memset(bitmap->data, color, bitmap->pitch * bitmap->height);
}

static bitmap_t *_make_bitmap(uint8 *data_addr, bool hw, int width,
                              int height, int pitch, int overdraw)
{
   bitmap_t *bitmap;
   int i;
 
   if (NULL == data_addr)
      return NULL;
 
   bitmap = NOFRENDO_MALLOC(sizeof(bitmap_t) + (sizeof(uint8 *) * height));
   if (NULL == bitmap)
      return NULL;

   bitmap->hardware = hw;
   bitmap->height = height;
   bitmap->width = width;
   bitmap->data = data_addr;
   bitmap->pitch = pitch + (overdraw * 2);
 
   if (false == bitmap->hardware)
   {
      bitmap->pitch = (bitmap->pitch + 3) & ~3;
      bitmap->line[0] = (uint8 *)(((uint32)bitmap->data + overdraw + 3) & ~3);
   }
   else
   {
      bitmap->line[0] = bitmap->data + overdraw;
   }

   for (i = 1; i < height; i++)
      bitmap->line[i] = bitmap->line[i - 1] + bitmap->pitch;

   return bitmap;
}
 
bitmap_t *bmp_create(int width, int height, int overdraw)
{
   uint8 *addr;
   int pitch;

   pitch = width + (overdraw * 2); 

   addr = NOFRENDO_MALLOC(((pitch * height) + 3) & 0xFFFFFFF8); 
   if (NULL == addr)
      return NULL;

   return _make_bitmap(addr, false, width, height, width, overdraw);
}
 
bitmap_t *bmp_createhw(uint8 *addr, int width, int height, int pitch)
{
   return _make_bitmap(addr, true, width, height, pitch, 0); 
}
 
void bmp_destroy(bitmap_t **bitmap)
{
   if (*bitmap)
   {
      if ((*bitmap)->data && false == (*bitmap)->hardware)
         NOFRENDO_FREE((*bitmap)->data);
      NOFRENDO_FREE(*bitmap);
      *bitmap = NULL;
   }
}