#ifndef _DIS6502_H_
#define _DIS6502_H_

#ifdef __cplusplus
extern "C"
{
#endif  

    extern char *nes6502_disasm(uint32 PC, uint8 P, uint8 A, uint8 X, uint8 Y, uint8 S);

#ifdef __cplusplus
}
#endif  

#endif  