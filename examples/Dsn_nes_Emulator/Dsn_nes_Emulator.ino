#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <TFT_eSPI.h>
#include "hw_config.h"

TFT_eSPI tft = TFT_eSPI();

void setup_controller();
extern "C" int nes_get_gamepad_state();
extern "C" int nofrendo_main(int argc, char *argv[]);
extern "C" bool vid_preload_rom(const char *path);

// Hardware Masks
#define HW_MASK_START 0x08
#define HW_MASK_UP 0x10
#define HW_MASK_DOWN 0x20
#define HW_MASK_A 0x01

char *gameToLoad = NULL;
TaskHandle_t emuTaskHandle = NULL;

void emulator_task(void *param) {
  char **argv = (char **)malloc(2 * sizeof(char *));
  argv[0] = strdup("nes");
  argv[1] = strdup("dummy.nes");

  Serial.println("[TASK] Starting Emulator...");
  nofrendo_main(2, argv);

  Serial.println("[TASK] Exited");
  vTaskDelete(NULL);
}

// Game Selection Menu
String selectGame() {
  File root = SD.open("/");
  if (!root) {
    tft.fillScreen(TFT_RED);
    tft.setCursor(0, 0);
    tft.println("SD Error");
    while (1) delay(100);
  }

  String files[20];
  int fileCount = 0;

  while (fileCount < 20) {
    File entry = root.openNextFile();
    if (!entry) break;
    String fn = entry.name();
    if (!entry.isDirectory() && (fn.endsWith(".nes") || fn.endsWith(".NES")) && !fn.startsWith(".")) {
      String path = fn;
      if (!path.startsWith("/")) path = "/" + path;
      files[fileCount++] = path;
    }
    entry.close();
  }
  root.close();

  if (fileCount == 0) return "";
  if (fileCount == 1) return files[0];

  int selected = 0;
  int prevSelected = -1;

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);

  while (1) {
    if (selected != prevSelected) {
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0);
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.println("NES Game Select:");
      tft.drawLine(0, 25, 320, 25, TFT_WHITE);
      tft.setCursor(0, 40);

      for (int i = 0; i < fileCount; i++) {
        if (i == selected) tft.setTextColor(TFT_BLACK, TFT_WHITE);
        else tft.setTextColor(TFT_WHITE, TFT_BLACK);

        tft.println(files[i].substring(1));
      }
      prevSelected = selected;
    }

    int input = nes_get_gamepad_state();

    if (input & HW_MASK_DOWN) {
      selected++;
      if (selected >= fileCount) selected = 0;
      delay(150);
    }
    if (input & HW_MASK_UP) {
      selected--;
      if (selected < 0) selected = fileCount - 1;
      delay(150);
    }
    if ((input & HW_MASK_START) || (input & HW_MASK_A)) {
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 100);
      tft.setTextDatum(MC_DATUM);
      tft.setTextColor(TFT_GREEN);
      tft.drawString("Loading...", 128, 120);
      tft.setTextDatum(TL_DATUM);
      return files[selected];
    }

    delay(50);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n=== ESP32-S3 NES Emulator v25.0 ===");

  setup_controller();

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  if (psramFound()) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("PSRAM OK");
  } else {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println("ERROR: No PSRAM");
    while (1) delay(1000);
  }

  Serial.println("[INIT] SD Card...");
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  if (!SD.begin(SD_CS, SPI, 10000000)) {
    if (!SD.begin(SD_CS, SPI, 4000000)) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("SD FAIL");
      while (1) delay(1000);
    }
  }

  String selectedGame = selectGame();

  if (selectedGame == "") {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println("NO GAMES FOUND");
    while (1) delay(1000);
  }

  gameToLoad = strdup(selectedGame.c_str());

  if (!vid_preload_rom(gameToLoad)) {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println("LOAD ERROR");
    while (1) delay(1000);
  }

  Serial.println("[INIT] Starting Task...");

  xTaskCreatePinnedToCore(
    emulator_task,
    "NES_Loop",
    64 * 1024,
    NULL,
    1,
    &emuTaskHandle,
    1);
}

void loop() {
  delay(2000);
}