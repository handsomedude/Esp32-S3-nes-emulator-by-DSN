#ifndef _NOFTYPES_H_
#define _NOFTYPES_H_

#include <stdbool.h>

#define NOFRENDO_DEBUG
#define NOFRENDO_MEM_DEBUG
// #define NOFRENDO_VRAM_DEBUG
// #define NOFRENDO_LOG_TO_FILE
/* For the ESP32, it costs too much memory to render to a separate buffer and blit that to the main buffer.
   Instead, the code has been modified to directly grab the primary buffer from the video subsystem and render
   there, saving us about 64K of memory. */
// #define NOFRENDO_DOUBLE_FRAMEBUFFER

/* Define this if running on little-endian (x86) systems */
#define HOST_LITTLE_ENDIAN

#ifdef __GNUC__
#define INLINE static inline
#define ZERO_LENGTH 0
#elif defined(WIN32)
#define INLINE static __inline
#define ZERO_LENGTH 0
#else /* crapintosh? */
#define INLINE static
#define ZERO_LENGTH 1
#endif

/* quell stupid compiler warnings */
#define UNUSED(x) ((x) = (x))

typedef signed char int8;
typedef signed short int16;
typedef signed int int32;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

#include "memguard.h"
#include "log.h"

#ifdef NOFRENDO_DEBUG

#define ASSERT(expr) nofrendo_log_assert((int)(expr), __LINE__, __FILE__, NULL)
#define ASSERT_MSG(msg) nofrendo_log_assert(false, __LINE__, __FILE__, (msg))

#else /* !NOFRENDO_DEBUG */

#define ASSERT(expr)
#define ASSERT_MSG(msg)

#endif /* !NOFRENDO_DEBUG */

#endif