#include "noftypes.h"
#include "nes_mmc.h"
#include "nes.h"
#include "libsnss.h"
#include "log.h"

static struct
{
  bool enabled;
  uint32 counter;
} irq;

/********************************/
/* Mapper #50 IRQ reset routine */
/********************************/
static void map50_irq_reset(void)
{
  /* Turn off IRQs */
  irq.enabled = false;
  irq.counter = 0x0000;

  /* Done */
  return;
}

/**************************************************************/
/* Mapper #50: 3rd discovered variation of SMB2j cart bootleg */
/**************************************************************/
static void map50_init(void)
{
  /* Set the hardwired pages */
  mmc_bankrom(8, 0x6000, 0x0F);
  mmc_bankrom(8, 0x8000, 0x08);
  mmc_bankrom(8, 0xA000, 0x09);
  mmc_bankrom(8, 0xE000, 0x0B);

  /* Reset the IRQ counter */
  map50_irq_reset();

  /* Done */
  return;
}

/****************************************/
/* Mapper #50 callback for IRQ handling */
/****************************************/
static void map50_hblank(int vblank)
{
  /* Counter is M2 based so it doesn't matter whether */
  /* the PPU is in its VBlank period or not           */
  UNUSED(vblank);

  /* Increment the counter if it is enabled and check for strike */
  if (irq.enabled)
  {
    /* Is there a constant for cycles per scanline? */
    /* If so, someone ought to substitute it here   */
    irq.counter = irq.counter + 114;

    /* IRQ line is hooked to Q12 of the counter */
    if (irq.counter & 0x1000)
    {
      /* Trigger the IRQ */
      nes_irq();

      /* Reset the counter */
      map50_irq_reset();
    }
  }
}

/******************************************/
/* Mapper #50 write handler ($4000-$5FFF) */
/******************************************/
static void map50_write(uint32 address, uint8 value)
{
  uint8 selectable_bank;

  /* For address to be decoded, A5 must be high and A6 low */
  if ((address & 0x60) != 0x20)
    return;

  /* A8 low  = $C000-$DFFF page selection */
  /* A8 high = IRQ timer toggle */
  if (address & 0x100)
  {
    /* IRQ settings */
    if (value & 0x01)
      irq.enabled = true;
    else
      map50_irq_reset();
  }
  else
  {
    /* Stupid data line swapping */
    selectable_bank = 0x00;
    if (value & 0x08)
      selectable_bank |= 0x08;
    if (value & 0x04)
      selectable_bank |= 0x02;
    if (value & 0x02)
      selectable_bank |= 0x01;
    if (value & 0x01)
      selectable_bank |= 0x04;
    mmc_bankrom(8, 0xC000, selectable_bank);
  }

  /* Done */
  return;
}

/****************************************************/
/* Shove extra mapper information into a SNSS block */
/****************************************************/
static void map50_setstate(SnssMapperBlock *state)
{
  /* TODO: Store SNSS information */
  UNUSED(state);

  /* Done */
  return;
}

/*****************************************************/
/* Pull extra mapper information out of a SNSS block */
/*****************************************************/
static void map50_getstate(SnssMapperBlock *state)
{
  /* TODO: Retrieve SNSS information */
  UNUSED(state);

  /* Done */
  return;
}

static map_memwrite map50_memwrite[] =
    {
        {0x4000, 0x5FFF, map50_write},
        {-1, -1, NULL}};

mapintf_t map50_intf =
    {
        50,                               /* Mapper number */
        "SMB2j (3rd discovered variant)", /* Mapper name */
        map50_init,                       /* Initialization routine */
        NULL,                             /* VBlank callback */
        map50_hblank,                     /* HBlank callback */
        map50_getstate,                   /* Get state (SNSS) */
        map50_setstate,                   /* Set state (SNSS) */
        NULL,                             /* Memory read structure */
        map50_memwrite,                   /* Memory write structure */
        NULL                              /* External sound device */
};