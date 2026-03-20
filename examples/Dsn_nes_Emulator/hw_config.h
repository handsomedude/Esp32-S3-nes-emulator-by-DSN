#ifndef HW_CONFIG_H
#define HW_CONFIG_H

// Set to 1 to enable sound, 0 to disable
#define ENABLE_SOUND 0

// SD Card Pins
#define SD_CS 21
#define SD_SCK 40
#define SD_MOSI 41
#define SD_MISO 39

// I2S Audio Pins (MAX98357A or similar DAC)
#define I2S_DO 35   // DIN on MAX98357A
#define I2S_BCK 36  // BCLK on MAX98357A
#define I2S_WS 37  // LRC on MAX98357A

// TFT Display Pins (ST7789)
#define HW_TFT_MOSI 11   // Data In
#define HW_TFT_SCK 12    // Clock
#define HW_TFT_CS 10     // Chip Select
#define HW_TFT_DC 9    // Data/Command
#define HW_TFT_RST 14     // Reset

// ST7789 display orientation (MADCTL)
// 0x00: normal
// 0x60: rotate 90°
// 0xC0: rotate 270°
// 0xA0: rotate 180° (vertical+horizontal flip)
// 0x20: horizontal mirror
// 0x40: vertical mirror
#define HW_TFT_MADCTL 0x20  // try 0x20 if text appears mirrored

// Controller/Button Pins (Active LOW with internal pullup)
#define BTN_UP 17
#define BTN_DOWN 16
#define BTN_LEFT 15
#define BTN_RIGHT 7
#define BTN_A 4
#define BTN_B 5
#define BTN_START 8
#define BTN_SELECT 0

#endif
