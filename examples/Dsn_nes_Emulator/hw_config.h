#ifndef HW_CONFIG_H
#define HW_CONFIG_H

// Set to 1 to enable sound, 0 to disable
#define ENABLE_SOUND 0

// SD Card Pins
#define SD_CS 41
#define SD_SCK 42
#define SD_MOSI 2
#define SD_MISO 1

// I2S Audio Pins (MAX98357A or similar DAC)
#define I2S_DO 4   // DIN on MAX98357A
#define I2S_BCK 5  // BCLK on MAX98357A
#define I2S_WS 6   // LRC on MAX98357A

// TFT Display Pins (ST7789)
#define HW_TFT_MOSI 11   // Data In
#define HW_TFT_SCK 12    // Clock
#define HW_TFT_CS 10     // Chip Select
#define HW_TFT_DC 15     // Data/Command
#define HW_TFT_RST 9     // Reset

// Controller/Button Pins (Active LOW with internal pullup)
#define BTN_UP 13
#define BTN_DOWN 14
#define BTN_LEFT 8
#define BTN_RIGHT 17
#define BTN_A 45
#define BTN_B 21
#define BTN_START 47
#define BTN_SELECT 39

#endif
