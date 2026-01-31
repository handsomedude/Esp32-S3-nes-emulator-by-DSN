#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SD.h>
#include <FS.h>
#include <esp_timer.h>
#include "hw_config.h"

extern "C" {
#include <noftypes.h>
#include <bitmap.h>
#include <osd.h>
#include <nofrendo.h>
#include <vid_drv.h>
#include <nesinput.h>
}

extern TFT_eSPI tft;
extern "C" int nes_get_gamepad_state();

#define NES_SCREEN_WIDTH 256
#define NES_SCREEN_HEIGHT 240

// --- INPUT DEFINITIONS ---
#define INP_JOYPAD0 0x0001
#define INP_JOYPAD1 0x0002

nesinput_t joypad_p1 = { INP_JOYPAD0, 0 };

// Hardware Masks
#define HW_MASK_A 0x01
#define HW_MASK_B 0x02
#define HW_MASK_SELECT 0x04
#define HW_MASK_START 0x08
#define HW_MASK_UP 0x10
#define HW_MASK_DOWN 0x20
#define HW_MASK_LEFT 0x40
#define HW_MASK_RIGHT 0x80

// NES Input Masks
#define INP_PAD_A 0x01
#define INP_PAD_B 0x02
#define INP_PAD_SELECT 0x04
#define INP_PAD_START 0x08
#define INP_PAD_UP 0x10
#define INP_PAD_DOWN 0x20
#define INP_PAD_LEFT 0x40
#define INP_PAD_RIGHT 0x80

#define INP_STATE_MAKE 1
#define INP_STATE_BREAK 0

uint16_t myPalette[256];
bitmap_t *game_bitmap = NULL;
uint32_t frame_count = 0;

// --- TIMER ---
esp_timer_handle_t nes_timer_handle = NULL;
void (*emu_timer_callback)(void) = NULL;

void IRAM_ATTR timer_callback_handler(void *arg) {
  if (emu_timer_callback) {
    emu_timer_callback();
  }
}

// --- BITMAP ALLOCATOR ---
bitmap_t *my_bmp_create(int w, int h) {
  size_t struct_size = sizeof(bitmap_t) + (h * sizeof(uint8 *));
  bitmap_t *b = (bitmap_t *)heap_caps_calloc(1, struct_size, MALLOC_CAP_SPIRAM);
  if (!b) b = (bitmap_t *)calloc(1, struct_size);
  if (!b) return NULL;

  b->width = w;
  b->height = h;
  b->data = (uint8 *)heap_caps_calloc(w * h, 1, MALLOC_CAP_SPIRAM);
  if (!b->data) b->data = (uint8 *)calloc(w * h, 1);
  if (!b->data) {
    free(b);
    return NULL;
  }

  for (int i = 0; i < h; i++) b->line[i] = &b->data[i * w];
  return b;
}

// --- ROM PRELOAD ---
char *global_rom_data = NULL;
int global_rom_size = 0;

extern "C" bool vid_preload_rom(const char *path) {
  if (global_rom_data) {
    free(global_rom_data);
    global_rom_data = NULL;
    global_rom_size = 0;
  }

  Serial.printf("[VID] Loading: %s\n", path);
  String fpath = String(path);
  if (!fpath.startsWith("/")) fpath = "/" + fpath;

  if (!SD.exists(fpath)) return false;
  File file = SD.open(fpath, FILE_READ);
  if (!file) return false;

  global_rom_size = file.size();
  if (global_rom_size <= 0) {
    file.close();
    return false;
  }

  if (psramFound()) global_rom_data = (char *)ps_malloc(global_rom_size);
  else global_rom_data = (char *)malloc(global_rom_size);

  if (!global_rom_data) {
    file.close();
    return false;
  }

  file.readBytes(global_rom_data, global_rom_size);
  file.close();
  Serial.printf("[VID] Loaded %d bytes\n", global_rom_size);
  return true;
}

// --- LOGGING ---
extern "C" int nofrendo_log_init(void) {
  return 0;
}
extern "C" void nofrendo_log_shutdown(void) {}
extern "C" int nofrendo_log_print(const char *s) {
  return 0;
}
extern "C" int nofrendo_log_printf(const char *format, ...) {
  return 0;
}
extern "C" void nofrendo_log_assert(int expr, int line, const char *file, char *msg) {
  if (!expr) Serial.printf("[EMU] ASSERT: %s:%d\n", file, line);
}

// --- MOCKS ---
extern "C" char *osd_newextension(char *string, char *ext) {
  return string;
}
extern "C" void osd_fullname(char *fullname, const char *shortname) {
  strcpy(fullname, shortname);
}

extern "C" char *osd_getromdata(char *name, int *size) {
  if (size == NULL || global_rom_data == NULL) return NULL;
  *size = global_rom_size;
  char *data = NULL;
  if (psramFound()) data = (char *)ps_malloc(*size);
  else data = (char *)malloc(*size);
  if (!data) return NULL;
  memcpy(data, global_rom_data, *size);
  Serial.println("[OSD] ROM Copy OK");
  return data;
}

// --- VIDEO ---
extern "C" void osd_initvideo(int *lines) {
  *lines = NES_SCREEN_HEIGHT;
}
extern "C" void osd_shutdownvideo() {}
extern "C" void osd_setscreen(int x, int y, int width, int height) {}

extern "C" void osd_setpalette(rgb_t *pal) {
  for (int i = 0; i < 256; i++) {
    // RGB Fix (Blue/Red swap)
    myPalette[i] = tft.color565(pal[i].g, pal[i].r, pal[i].b);
  }
}

extern "C" void osd_blit(bitmap_t *bmp) {
  if (!bmp || !bmp->line) return;

  static uint16_t lineBuffer[NES_SCREEN_WIDTH];
  int x_offset = (320 - NES_SCREEN_WIDTH) / 2;

  tft.startWrite();
  tft.setAddrWindow(x_offset, 0, NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT);

  for (int y = 0; y < NES_SCREEN_HEIGHT; y++) {
    uint8_t *src = bmp->line[y];
    for (int x = 0; x < NES_SCREEN_WIDTH; x++) {
      lineBuffer[x] = myPalette[src[x]];
    }
    tft.pushPixels(lineBuffer, NES_SCREEN_WIDTH);
  }
  tft.endWrite();

  frame_count++;
  delay(1);
}

extern "C" void osd_getvideoinfo(vidinfo_t *info) {
  info->default_width = NES_SCREEN_WIDTH;
  info->default_height = NES_SCREEN_HEIGHT;
  info->driver = 0;
}
extern "C" void osd_togglefullscreen(int code) {}

// --- AUDIO SYSTEM (ROBUST PWM) ---
#define AUDIO_SAMPLE_RATE 22050  // Nofrendo Standard
#define AUDIO_BUF_SIZE 4096

volatile uint8_t audio_buffer[AUDIO_BUF_SIZE];
volatile int audio_head = 0;
volatile int audio_tail = 0;
esp_timer_handle_t audio_timer = NULL;

void IRAM_ATTR onAudioTimer(void *arg) {
  if (audio_head != audio_tail) {
    uint8_t sample = audio_buffer[audio_tail];
    audio_tail = (audio_tail + 1) % AUDIO_BUF_SIZE;
 
    ledcWrite(AUDIO_PIN, sample);
  }
}

extern "C" void osd_setsound(void (*playfunc)(void *buffer, int length)) {}

extern "C" int osd_init_sound(void) {
  Serial.println("[OSD] Audio Init...");
 
  if (!ledcAttach(AUDIO_PIN, 1000, 8)) {  // 1kHz
    Serial.println("[OSD] LEDC Attach Failed!");
    return -1;
  }

  // Play BEEP
  ledcWrite(AUDIO_PIN, 127);  // 50% duty
  delay(200);
  ledcWrite(AUDIO_PIN, 0);  // Silence

  // 2. Switch to Ultrasonic Carrier Frequency (for PCM Audio)
  // 80kHz carrier acts as a poor man's DAC
  if (!ledcChangeFrequency(AUDIO_PIN, 80000, 8)) {
    // Fallback if change fails: detach and re-attach
    ledcDetach(AUDIO_PIN);
    ledcAttach(AUDIO_PIN, 80000, 8);
  }

  // Ensure silence initially
  ledcWrite(AUDIO_PIN, 0);

  // 3. Start Buffer Timer
  const esp_timer_create_args_t audio_timer_args = {
    .callback = &onAudioTimer,
    .name = "audio_timer"
  };

  esp_err_t err = esp_timer_create(&audio_timer_args, &audio_timer);
  if (err == ESP_OK) {
    esp_timer_start_periodic(audio_timer, 1000000 / AUDIO_SAMPLE_RATE);
    Serial.println("[OSD] Audio Timer OK");
  } else {
    Serial.println("[OSD] Audio Timer FAIL");
  }

  return 0;
}

extern "C" void osd_stopsound(void) {
  if (audio_timer) esp_timer_stop(audio_timer);
  ledcWrite(AUDIO_PIN, 0);
}

extern "C" void osd_writesound(void *stream, int len) {
  // Input: 16-bit signed PCM from emulator
  int16_t *data = (int16_t *)stream;
  int samples = len / 2;

  // Debug: Check if sound data is arriving
  static bool sound_active = false;
  if (!sound_active && samples > 0) {
    Serial.println("[OSD] Sound Data Streaming...");
    sound_active = true;
  }

  for (int i = 0; i < samples; i++) {
    int next_head = (audio_head + 1) % AUDIO_BUF_SIZE;

    if (next_head != audio_tail) {
      // Convert: Signed 16-bit -> Unsigned 8-bit
      int32_t val = (int32_t)data[i];

      // Amplify: Shift less or multiply?
      // Standard: +32768 >> 8.
      // Boost volume for buzzer: map to 8-bit directly with clipping
      val = val + 32768;  // 0..65535
      val = val >> 8;     // 0..255

      // Basic clipping just in case
      if (val < 0) val = 0;
      if (val > 255) val = 255;

      audio_buffer[audio_head] = (uint8_t)val;
      audio_head = next_head;
    } else {
      break;
    }
  }
}

extern "C" void osd_getsoundinfo(sndinfo_t *info) {
  info->sample_rate = AUDIO_SAMPLE_RATE;
  info->bps = 16;
}

// --- SYSTEM ---

extern "C" int osd_init() {
  Serial.println("[OSD] osd_init called");
  input_register(&joypad_p1);

  osd_init_sound();  // Start Audio

  Serial.println("[OSD] Player 1 Registered!");
  return 0;
}

extern "C" void osd_shutdown() {
  if (nes_timer_handle) {
    esp_timer_stop(nes_timer_handle);
    esp_timer_delete(nes_timer_handle);
    nes_timer_handle = NULL;
  }
  osd_stopsound();
}
extern "C" int osd_gettime(void) {
  return millis();
}

extern "C" int osd_installtimer(int freq, void *func, int func_param, void *func2, int func2_param) {
  Serial.printf("[OSD] Installing HW Timer: %d Hz\n", freq);
  emu_timer_callback = (void (*)(void))func;
  if (nes_timer_handle) {
    esp_timer_stop(nes_timer_handle);
    esp_timer_delete(nes_timer_handle);
  }
  const esp_timer_create_args_t timer_args = { .callback = &timer_callback_handler, .name = "nes_timer" };
  esp_timer_create(&timer_args, &nes_timer_handle);
  esp_timer_start_periodic(nes_timer_handle, 1000000 / freq);
  return 0;
}

extern "C" int osd_makesnapname(char *buf, int len) {
  strncpy(buf, "/mario.sav", len);
  return 0;
}

extern "C" int osd_main(int argc, char *argv[]) {
  return main_loop(argv[0], system_nes);
}

// --- MISSING EVENTS ---
extern "C" {
  void event_init(void) {
    input_register(&joypad_p1);
  }
  void event_set_system(int system) {}
  typedef void (*event_func_t)(int);
  event_func_t event_get(int index) {
    return NULL;
  }
}

// --- INPUT HANDLER ---
extern "C" void osd_getinput(void) {
  int hw = nes_get_gamepad_state();

  int nes_data = 0;
  if (hw & HW_MASK_A) nes_data |= INP_PAD_A;
  if (hw & HW_MASK_B) nes_data |= INP_PAD_B;
  if (hw & HW_MASK_SELECT) nes_data |= INP_PAD_SELECT;
  if (hw & HW_MASK_START) nes_data |= INP_PAD_START;
  if (hw & HW_MASK_UP) nes_data |= INP_PAD_UP;
  if (hw & HW_MASK_DOWN) nes_data |= INP_PAD_DOWN;
  if (hw & HW_MASK_LEFT) nes_data |= INP_PAD_LEFT;
  if (hw & HW_MASK_RIGHT) nes_data |= INP_PAD_RIGHT;

  joypad_p1.data = nes_data;
}

extern "C" void osd_getmouse(int *x, int *y, int *button) {
  *x = 0;
  *y = 0;
  *button = 0;
}

// --- VID_DRV INTERFACE ---
extern "C" int vid_init(int width, int height, viddriver_t *osd_driver) {
  if (game_bitmap == NULL) game_bitmap = my_bmp_create(NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT);
  return 0;
}
extern "C" void vid_shutdown() {
  if (game_bitmap) {
    free(game_bitmap->data);
    free(game_bitmap);
    game_bitmap = NULL;
  }
}
extern "C" int vid_setmode(int width, int height) {
  return 0;
}
extern "C" bitmap_t *vid_getbuffer() {
  return game_bitmap;
}
extern "C" void vid_flush() {
  if (game_bitmap) osd_blit(game_bitmap);
}
extern "C" void vid_setpalette(rgb_t *pal) {
  osd_setpalette(pal);
}