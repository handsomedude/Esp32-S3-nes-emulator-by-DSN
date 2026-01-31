#include "noftypes.h"
#include "nes_mmc.h"
#include "nes.h"
#include "libsnss.h"
#include "log.h"

/************************/
/* Mapper #229: 31 in 1 */
/************************/
static void map229_init(void)
{
  /* On reset, PRG is set to first 32K and CHR to first 8K */
  mmc_bankrom(32, 0x8000, 0x00);
  mmc_bankvrom(8, 0x0000, 0x00);

  /* Done */
  return;
}

/*******************************************/
/* Mapper #229 write handler ($8000-$FFFF) */
/*******************************************/
static void map229_write(uint32 address, uint8 value)
{
  /* Value written is irrelevant */
  UNUSED(value);

  /* A4-A0 sets 8K CHR page */
  mmc_bankvrom(8, 0x0000, (uint8)(address & 0x1F));

  /* If A4-A1 are all low then select the first 32K,     */
  /* otherwise select a 16K bank at both $8000 and $C000 */
  if ((address & 0x1E) == 0x00)
  {
    mmc_bankrom(32, 0x8000, 0x00);
  }
  else
  {
    mmc_bankrom(16, 0x8000, (uint8)(address & 0x1F));
    mmc_bankrom(16, 0xC000, (uint8)(address & 0x1F));
  }

  /* A5: mirroring (low = vertical, high = horizontal) */
  if (address & 0x20)
    ppu_mirror(0, 0, 1, 1);
  else
    ppu_mirror(0, 1, 0, 1);

  /* Done */
  return;
}

static map_memwrite map229_memwrite[] =
    {
        {0x8000, 0xFFFF, map229_write},
        {-1, -1, NULL}};

mapintf_t map229_intf =
    {
        229,                 /* Mapper number */
        "31 in 1 (bootleg)", /* Mapper name */
        map229_init,         /* Initialization routine */
        NULL,                /* VBlank callback */
        NULL,                /* HBlank callback */
        NULL,                /* Get state (SNSS) */
        NULL,                /* Set state (SNSS) */
        NULL,                /* Memory read structure */
        map229_memwrite,     /* Memory write structure */
        NULL                 /* External sound device */
};