#include "noftypes.h"
#include "nes_mmc.h"
#include "nes_ppu.h"

/* mapper 78: Holy Diver, Cosmo Carrier */
/* ($8000-$FFFF) D2-D0 = switch $8000-$BFFF */
/* ($8000-$FFFF) D7-D4 = switch PPU $0000-$1FFF */
/* ($8000-$FFFF) D3 = switch mirroring */
static void map78_write(uint32 address, uint8 value)
{
   UNUSED(address);

   mmc_bankrom(16, 0x8000, value & 7);
   mmc_bankvrom(8, 0x0000, (value >> 4) & 0x0F);

   /* Ugh! Same abuse of the 4-screen bit as with Mapper #70 */
   if (mmc_getinfo()->flags & ROM_FLAG_FOURSCREEN)
   {
      if (value & 0x08)
         ppu_mirror(0, 1, 0, 1); /* vert */
      else
         ppu_mirror(0, 0, 1, 1); /* horiz */
   }
   else
   {
      int mirror = (value >> 3) & 1;
      ppu_mirror(mirror, mirror, mirror, mirror);
   }
}

static map_memwrite map78_memwrite[] =
    {
        {0x8000, 0xFFFF, map78_write},
        {-1, -1, NULL}};

mapintf_t map78_intf =
    {
        78,             /* mapper number */
        "Mapper 78",    /* mapper name */
        NULL,           /* init routine */
        NULL,           /* vblank callback */
        NULL,           /* hblank callback */
        NULL,           /* get state (snss) */
        NULL,           /* set state (snss) */
        NULL,           /* memory read structure */
        map78_memwrite, /* memory write structure */
        NULL            /* external sound device */
};