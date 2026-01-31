#include "noftypes.h"
#include "nes_mmc.h"
#include "nes_ppu.h"

/* mapper 70: Arkanoid II, Kamen Rider Club, etc. */
/* ($8000-$FFFF) D6-D4 = switch $8000-$BFFF */
/* ($8000-$FFFF) D3-D0 = switch PPU $0000-$1FFF */
/* ($8000-$FFFF) D7 = switch mirroring */
static void map70_write(uint32 address, uint8 value)
{
   UNUSED(address);

   mmc_bankrom(16, 0x8000, (value >> 4) & 0x07);
   mmc_bankvrom(8, 0x0000, value & 0x0F);
 
   if (mmc_getinfo()->flags & ROM_FLAG_FOURSCREEN)
   {
      if (value & 0x80)
         ppu_mirror(0, 0, 1, 1); /* horiz */
      else
         ppu_mirror(0, 1, 0, 1); /* vert */
   }
   else
   {
      int mirror = (value & 0x80) >> 7;
      ppu_mirror(mirror, mirror, mirror, mirror);
   }
}

static map_memwrite map70_memwrite[] =
    {
        {0x8000, 0xFFFF, map70_write},
        {-1, -1, NULL}};

mapintf_t map70_intf =
    {
        70,             /* mapper number */
        "Mapper 70",    /* mapper name */
        NULL,           /* init routine */
        NULL,           /* vblank callback */
        NULL,           /* hblank callback */
        NULL,           /* get state (snss) */
        NULL,           /* set state (snss) */
        NULL,           /* memory read structure */
        map70_memwrite, /* memory write structure */
        NULL            /* external sound device */
};