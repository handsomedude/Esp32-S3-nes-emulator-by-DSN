#ifndef _NES_H_
#define _NES_H_

#include <noftypes.h>
#include <nes_apu.h>
#include <nes_mmc.h>
#include <nes_ppu.h>
#include <nes_rom.h>
#include "nes6502.h"
#include <bitmap.h>

/* Visible (NTSC) screen height */
#ifndef NES_VISIBLE_HEIGHT
#define  NES_VISIBLE_HEIGHT   224
#endif /* !NES_VISIBLE_HEIGHT */
#define  NES_SCREEN_WIDTH     256
#define  NES_SCREEN_HEIGHT    240

/* NTSC = 60Hz, PAL = 50Hz */
#ifdef PAL
#define  NES_REFRESH_RATE     50
#else /* !PAL */
#define  NES_REFRESH_RATE     60
#endif /* !PAL */

#define  MAX_MEM_HANDLERS     32

enum
{
   SOFT_RESET,
   HARD_RESET
};


typedef struct nes_s
{
   /* hardware things */
   nes6502_context *cpu;
   nes6502_memread readhandler[MAX_MEM_HANDLERS];
   nes6502_memwrite writehandler[MAX_MEM_HANDLERS];

   ppu_t *ppu;
   apu_t *apu;
   mmc_t *mmc;
   rominfo_t *rominfo;
 
//   bitmap_t *vidbuf; 

   bool fiq_occurred;
   uint8 fiq_state;
   int fiq_cycles;

   int scanline;

   /* Timing stuff */
   float scanline_cycles;
   bool autoframeskip;

   /* control */
   bool poweroff;
   bool pause;

} nes_t;


extern int nes_isourfile(const char *filename);

/* temp hack */
extern nes_t *nes_getcontextptr(void);

/* Function prototypes */
extern void nes_getcontext(nes_t *machine);
extern void nes_setcontext(nes_t *machine);

extern nes_t *nes_create(void);
extern void nes_destroy(nes_t **machine);
extern int nes_insertcart(const char *filename, nes_t *machine);

extern void nes_setfiq(uint8 state);
extern void nes_nmi(void);
extern void nes_irq(void);
extern void nes_emulate(void);

extern void nes_reset(int reset_type);

extern void nes_poweroff(void);
extern void nes_togglepause(void);

#endif