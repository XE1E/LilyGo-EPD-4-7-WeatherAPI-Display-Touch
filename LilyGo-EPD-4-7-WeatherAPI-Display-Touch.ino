// ESP32 Weather Display and a LilyGo EPD 4.7" Display, obtains Open Weather Map data, decodes and then displays it.
// This software, the ideas and concepts is Copyright (c) David Bird 2021. All rights to this software are reserved.
// Mods by XE1E
// #################################################################################################################

// @note      Arduino Setting 
//            Tools ->
//                  Board:"ESP32S3 Dev Module"
//                  USB CDC On Boot:"Enable"
//                  USB DFU On Boot:"Disable"
//                  Flash Size : "16MB(128Mb)"
//                  Flash Mode"QIO 80MHz
//                  Partition Scheme:"16M Flash(3M APP/9.9MB FATFS)"
//                  PSRAM:"OPI PSRAM"
//                  Upload Mode:"UART0/Hardware CDC"
//                  USB Mode:"Hardware CDC and JTAG"
//            Board Manager ->
//                  esp32 by Espressif Systems 2.0.17
//            Libraries ->
//                  EPD47-master https://github.com/DFRobotdl/EPD47/archive/refs/heads/master.zip
//                  Arduinojson by Benoit Blanchon 6.19.0
//                  put only this 2 libraries in libraries folder
//            To force upload mode ->
//                  Press and hold the BOOT(IO0) button
//                  While still pressing the BOOT(IO0) button, press RST
//                  Release the RST
//                  Release the BOOT(IO0) button
//
//

#include <Arduino.h>            // In-built
#include <esp_task_wdt.h>       // In-built
#include <esp_sleep.h>          // In-built - for deep sleep and wake-up
#include "freertos/FreeRTOS.h"  // In-built
#include "freertos/task.h"      // In-built
#include "epd_driver.h"         // https://github.com/Xinyuan-LilyGO/LilyGo-EPD47
#include "esp_adc_cal.h"        // In-built

#include <ArduinoJson.h>        // https://github.com/bblanchon/ArduinoJson
#include <HTTPClient.h>         // In-built

#include <WiFi.h>               // In-built
#include <WiFiClientSecure.h>   // In-built - for HTTPS (WeatherAPI requires HTTPS)
#include <ArduinoOTA.h>         // In-built - for OTA updates from Arduino IDE
#include <SPI.h>                // In-built
#include <SD.h>                 // In-built - for SD card storage
#include <time.h>               // In-built
#include <FFat.h>               // In-built - for weather history storage (uses FATFS partition)
#include <qrcode.h>             // QR code generation for info screen

// SD Card pins for LilyGo T5 4.7" S3 (from official LilyGo-EPD47 repo)
#define SD_CS    42
#define SD_MOSI  15
#define SD_MISO  16
#define SD_CLK   11
bool sdCardAvailable = false;

#include "owm_credentials.h"
#include "forecast_record.h"
#include "weather_history.h"    // Weather history storage (SD + FFat)
#include "wifi_manager.h"       // AP mode and captive portal (needs weather_history.h)
#include "lang.h"
#include "sunset.h"
#include "sunrise.h"
#include "moon.h"

#define SCREEN_WIDTH   EPD_WIDTH
#define SCREEN_HEIGHT  EPD_HEIGHT

//################  VERSION  ##################################################
String version = "2.9 / 4.7in";  // Programme version, see CHANGELOG.md
//################ VARIABLES ##################################################

enum alignment {LEFT, RIGHT, CENTER};
#define White         0xFF
#define LightGrey     0xBB
#define Grey          0x88
#define DarkGrey      0x44
#define Black         0x00

#define autoscale_on  true
#define autoscale_off false
#define barchart_on   true
#define barchart_off  false

boolean LargeIcon   = true;
boolean SmallIcon   = false;
#define Small  8            // For icon drawing
#define Large  20           // For icon drawing
#define XLarge 25           // Extra large for secondary screen
int iconScaleOverride = 0;  // 0 = use default, >0 = use this scale
String  Time_str = "--:--:--";
String  Date_str = "-- --- ----";
// Last successful weather update (survives deep sleep)
RTC_DATA_ATTR char LastUpdateTime[12] = "--:--:--";
RTC_DATA_ATTR char LastUpdateDate[32] = "-- --- ----";
RTC_DATA_ATTR bool hasValidWeatherData = false;
// Station info from last API response
RTC_DATA_ATTR char StationName[50] = "";
RTC_DATA_ATTR uint32_t StationDataEpoch = 0;  // localtime_epoch from API
RTC_DATA_ATTR float StationLat = 0;
RTC_DATA_ATTR float StationLon = 0;
int     wifi_signal, CurrentHour = 0, CurrentMin = 0, CurrentSec = 0, vref = 1100;

// Unit conversion constants
#define HPA_TO_INHG 0.02953
#define MM_TO_INCHES 0.0393701

//################ PROGRAM VARIABLES and OBJECTS ##########################################
#define max_readings 40 // Buffer size (array capacity)
#define forecast_readings 24 // WeatherAPI gives 3 days = 24 readings (8 per day)

Forecast_record_type  WxConditions[1];
Forecast_record_type  WxForecast[max_readings];

float pressure_readings[max_readings]    = {0};
float temperature_readings[max_readings] = {0};
float humidity_readings[max_readings]    = {0};
float rain_readings[max_readings]        = {0};
float snow_readings[max_readings]        = {0};

long SleepDuration   = 10; // Sleep time in minutes, aligned to the nearest minute boundary, so if 30 will always update at 00 or 30 past the hour
int  WakeupHour      = 8;  // Don't wakeup until after 07:00 to save battery power
int  SleepHour       = 1; // Sleep after 01:00 to save battery power
long StartTime       = 0;
long SleepTimer      = 0;
long Delta           = 30; // ESP32 rtc speed compensation, prevents display at xx:59:yy and then xx:00:yy (one minute later) to save power

// Timezone offset between API and device (in hours)
int apiTimezoneOffset = 0;
String adjustTimeByOffset(String timeStr, int offsetHours);  // Forward declaration

// AP Mode state
bool inAPMode = false;
const bool FORCE_AP_MODE = false;  // Set to true to force AP mode on next boot
unsigned long apModeStartTime = 0;
const unsigned long AP_RETRY_TIMEOUT = 300000;  // 5 minutes timeout (only for recovery mode)
const int AP_MAX_RETRIES = 5;  // Max WiFi connection attempts before permanent sleep
int apRetryCount = 0;

// Helper to check if AP mode should have timeout
bool apModeHasTimeout() {
  // Only recovery mode has timeout; initial setup and forced mode don't
  return currentAPModeType == AP_RECOVERY;
}


//fonts
#include "opensans6.h"
#include "opensans8b.h"
#include "opensans9b.h"
#include "opensans10b.h"
#include "opensans12b.h"
#include "opensans14b.h"
#include "opensans16b.h"
#include "opensans18b.h"
#include "opensans24b.h"

// Touch navigation (screen functions defined later in this file)
#include "touch_handler.h"
#include "calendar.h"

// Forward declarations for screen functions
void DisplayCurrentDetailScreen();
void DisplayAirQualityScreen();
void DisplayForecastScreen();
void DisplayGraphsScreen();
void DisplayInfoScreen();
void DisplayInfoFeatures1Screen();
void DisplayInfoFeatures2Screen();
void DisplayInfoUpdateScreen();
void DisplayInfoHelpScreen();
void DisplayInfoCreditsScreen();
void DisplayHistoryScreen();
void DisplayWeather();
void DisplayConditionsSection(int x, int y, String IconName, bool IconSize);
void deepCleanDisplay();
String WindDegToOrdinalDirection(float winddirection);
String TitleCase(String text);
String ConvertUnixTime(int unix_time);
void DrawGraph(int x_pos, int y_pos, int gwidth, int gheight, float Y1Min, float Y1Max, String title, float DataArray[], int readings, boolean auto_scale, boolean barchart_mode, int title_x_offset = 0, int hours_span = 0);
void DrawHistoryGraph(int x_pos, int y_pos, int gwidth, int gheight, float Y1Min, float Y1Max, String title, String unit, float DataArray[], float PosArray[], int readings, boolean auto_scale, boolean barchart_mode, int hours_span, boolean weekly_view, int start_hour);
void DrawSDCard(int x, int y);
float SumOfPrecip(float DataArray[], int readings);
void drawString(int x, int y, String text, alignment align);
int getTextWidthPixels(String text);
int drawWrappedText(int centerX, int startY, int maxWidth, int lineHeight, const char* text, int maxLines);
void drawScreenHeader();
void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void fillCircle(int x, int y, int r, uint8_t color);
void drawCircle(int x0, int y0, int r, uint8_t color);
void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void drawFastHLine(int16_t x0, int16_t y0, int length, uint16_t color);
void drawFastVLine(int16_t x0, int16_t y0, int length, uint16_t color);
void setFont(GFXfont const &font);
void edp_update();
#include "qr_codes.h"
#include "callsign.h"
#include "ble_config.h"
#include "worldclock.h"
#include "radio_screens.h"
#include "quote_screen.h"
#include "weather_narrative.h"

GFXfont  currentFont;
uint8_t *framebuffer;

void BeginSleep() {
  // Don't sleep while navigating secondary screens
  if (navigationModeActive) {
    Serial.println("Navigation active - skipping sleep");
    return;
  }

  // Don't sleep while web config is being accessed
  if (isWebActive()) {
    Serial.println("Web config active - skipping sleep");
    return;
  }

  // Stop web server and WiFi before sleep
  stopWebServer();
  StopWiFi();

  // Save current screen to NVS (RTC_DATA_ATTR doesn't work reliably on ESP32-S3)
  preferences.begin("weather", false);
  preferences.putInt("lastscreen", (int)currentScreen);
  preferences.end();

  displayPowerOff();
  UpdateLocalTime();
  SleepTimer = (SleepDuration * 60 - ((CurrentMin % SleepDuration) * 60 + CurrentSec)) + Delta; //Some ESP32 have a RTC that is too fast to maintain accurate time, so add an offset

  // Enable wake-up by timer
  esp_sleep_enable_timer_wakeup(SleepTimer * 1000000LL); // in Secs, 1000000LL converts to Secs as unit = 1uSec

  Serial.println("Awake for : " + String((millis() - StartTime) / 1000.0, 3) + "-secs");
  Serial.println("Entering " + String(SleepTimer) + " (secs) of sleep time");
  Serial.println("Starting deep-sleep period...");
  esp_deep_sleep_start();  // Sleep for e.g. 30 minutes
}

void setupArduinoOTA() {
  ArduinoOTA.setHostname("WeatherStation");

  ArduinoOTA.onStart([]() {
    String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
    Serial.println("OTA Start: " + type);
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA End - Rebooting...");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();
  Serial.println("ArduinoOTA ready. Hostname: WeatherStation");
}

boolean SetupTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, "time.nist.gov"); //(gmtOffset_sec, daylightOffset_sec, ntpServer)
  setenv("TZ", Timezone, 1);  //setenv()adds the "TZ" variable to the environment with a value TimeZone, only used if set to 1, 0 means no change
  tzset(); // Set the TZ environment variable
  delay(100);
  return UpdateLocalTime();
}

uint8_t StartWiFi() {
  WiFi.disconnect();
  WiFi.mode(WIFI_STA); // switch off AP
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);

  // Build network list from stored config + hardcoded credentials
  WiFiCredentialEntry configNetworks[6];
  int configNetworkCount = getConfiguredNetworks(configNetworks, 3);

  // Scan for available networks
  Serial.println("\r\nScanning for WiFi networks...");
  int networksFound = WiFi.scanNetworks();
  Serial.printf("Found %d networks\n", networksFound);

  // Find the best network (highest RSSI) among configured SSIDs
  int bestNetworkIndex = -1;
  int bestRSSI = -999;
  bool useStoredConfig = false;

  for (int i = 0; i < networksFound; i++) {
    String foundSSID = WiFi.SSID(i);
    int foundRSSI = WiFi.RSSI(i);
    Serial.printf("  %s (%d dBm)\n", foundSSID.c_str(), foundRSSI);

    // First check stored config networks
    for (int j = 0; j < configNetworkCount; j++) {
      if (foundSSID == configNetworks[j].ssid) {
        Serial.printf("    -> Stored config network found!\n");
        if (foundRSSI > bestRSSI) {
          bestRSSI = foundRSSI;
          bestNetworkIndex = j;
          useStoredConfig = true;
        }
        break;
      }
    }

    // Then check hardcoded networks (lower priority)
    if (!useStoredConfig || foundRSSI > bestRSSI) {
      for (int j = 0; j < wifiNetworkCount; j++) {
        if (foundSSID == wifiNetworks[j].ssid) {
          Serial.printf("    -> Hardcoded network found!\n");
          if (foundRSSI > bestRSSI) {
            bestRSSI = foundRSSI;
            bestNetworkIndex = j;
            useStoredConfig = false;
          }
          break;
        }
      }
    }
  }

  WiFi.scanDelete(); // Free memory from scan results

  // Connect to the best network found
  const char* selectedSSID = nullptr;
  const char* selectedPass = nullptr;

  if (bestNetworkIndex >= 0) {
    if (useStoredConfig) {
      selectedSSID = configNetworks[bestNetworkIndex].ssid;
      selectedPass = configNetworks[bestNetworkIndex].password;
    } else {
      selectedSSID = wifiNetworks[bestNetworkIndex].ssid;
      selectedPass = wifiNetworks[bestNetworkIndex].password;
    }
    Serial.printf("Connecting to: %s (signal: %d dBm)\n", selectedSSID, bestRSSI);
    WiFi.begin(selectedSSID, selectedPass);

    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.printf("STA: Failed!\n");
      WiFi.disconnect(false);
      delay(200);  // Reduced from 500ms
      WiFi.begin(selectedSSID, selectedPass);
      WiFi.waitForConnectResult();
    }
  } else {
    Serial.println("No configured networks found in scan results!");
    // Try stored config first, then hardcoded as fallback
    if (configNetworkCount > 0) {
      Serial.printf("Trying stored config: %s\n", configNetworks[0].ssid);
      WiFi.begin(configNetworks[0].ssid, configNetworks[0].password);
      WiFi.waitForConnectResult();
    }
    if (WiFi.status() != WL_CONNECTED && wifiNetworkCount > 0) {
      Serial.printf("Trying hardcoded fallback: %s\n", wifiNetworks[0].ssid);
      WiFi.begin(wifiNetworks[0].ssid, wifiNetworks[0].password);
      WiFi.waitForConnectResult();
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifi_signal = WiFi.RSSI(); // Get Wifi Signal strength now, because the WiFi will be turned off to save power!
    Serial.println("WiFi connected at: " + WiFi.localIP().toString());
  }
  else {
    Serial.println("WiFi connection *** FAILED ***");
  }
  return WiFi.status();
}

void StopWiFi() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  Serial.println("WiFi switched Off");
}


void InitialiseSystem() {
  StartTime = millis();
  Serial.begin(115200);
  delay(200); // Reduced from 1000ms - enough for serial init
  Serial.println(String(__FILE__) + "\nStarting...");
  epd_init();
  framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
  if (!framebuffer) Serial.println("Memory alloc failed!");
  memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
}

// Initialize SD card
bool initSDCard() {
  SPI.begin(SD_CLK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS)) {
    Serial.println("SD Card: Not found or failed to mount");
    return false;
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("SD Card: No card inserted");
    return false;
  }

  String type = "UNKNOWN";
  if (cardType == CARD_MMC) type = "MMC";
  else if (cardType == CARD_SD) type = "SD";
  else if (cardType == CARD_SDHC) type = "SDHC";

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card: %s, %lluMB\n", type.c_str(), cardSize);

  return true;
}

// Track if display is powered on for navigation
bool displayPoweredOn = false;

// Draw current screen content to framebuffer
void drawCurrentScreen() {
  switch (currentScreen) {
    case SCREEN_MAIN:           DisplayWeather(); break;
    case SCREEN_CURRENT_DETAIL: DisplayCurrentDetailScreen(); break;
    case SCREEN_FORECAST:       DisplayForecastScreen(); break;
    case SCREEN_GRAPHS:         DisplayGraphsScreen(); break;
    case SCREEN_INFO:           DisplayInfoScreen(); break;
    case SCREEN_HISTORY:        DisplayHistoryScreen(); break;
    case SCREEN_AIR_QUALITY:    DisplayAirQualityScreen(); break;
    case SCREEN_CALENDAR:       DisplayCalendarScreen(); break;
    case SCREEN_CALENDAR_YEAR:  DisplayCalendarYearScreen(); break;
    case SCREEN_INFO_FEATURES1: DisplayInfoFeatures1Screen(); break;
    case SCREEN_INFO_FEATURES2: DisplayInfoFeatures2Screen(); break;
    case SCREEN_INFO_UPDATE:    DisplayInfoUpdateScreen(); break;
    case SCREEN_INFO_HELP:      DisplayInfoHelpScreen(); break;
    case SCREEN_INFO_CREDITS:   DisplayInfoCreditsScreen(); break;
    case SCREEN_CALLSIGN:       DisplayCallsignScreen(); break;
    case SCREEN_BLE_CONFIG:     enterBLEConfigMode(); break;
    case SCREEN_WORLDCLOCK:     DisplayWorldClockScreen(); break;
    case SCREEN_RADIO_MENU:     DisplayRadioMenuScreen(); break;
    case SCREEN_RADIO_PHONETIC: DisplayRadioPhoneticScreen(); break;
    case SCREEN_RADIO_QCODES:   DisplayRadioQCodesScreen(); break;
    case SCREEN_RADIO_MORSE:    DisplayRadioMorseScreen(); break;
    case SCREEN_RADIO_DXCC:     DisplayRadioDXCCScreen(); break;
    case SCREEN_RADIO_DXCC2:    DisplayRadioDXCC2Screen(); break;
    case SCREEN_RADIO_DXCC3:    DisplayRadioDXCC3Screen(); break;
    case SCREEN_RADIO_PROPAGATION: DisplayRadioPropagationScreen(); break;
    case SCREEN_RADIO_CONTESTS: DisplayRadioContestsScreen(); break;
    case SCREEN_QUOTE: DisplayQuoteScreen(); break;
    case SCREEN_WEATHER_NARRATIVE: DisplayWeatherNarrativeScreen(); break;
  }
}

// Enter BLE configuration mode from menu
void enterBLEConfigMode() {
  Serial.println("Entering BLE config mode from menu...");
  epd_clear();
  delay(100);
  DisplayBLEConfigScreen();
  delay(500);
  initBLE();

  // Stay in BLE mode until restart
  while (bleConfigMode) {
    if (bleDeviceConnected != bleOldDeviceConnected) {
      bleOldDeviceConnected = bleDeviceConnected;
      epd_clear();  // Clear ghosting before redraw
      DisplayBLEConfigScreen();
    }
    delay(100);
  }
}

// Fast screen refresh - no epd_clear(), just overwrite framebuffer
void refreshScreenFast() {
  if (!displayPoweredOn) {
    epd_poweron();
    displayPoweredOn = true;
  }
  memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
  drawCurrentScreen();
  edp_update();
}

// Full screen refresh with physical clear (use sparingly)
void refreshScreen() {
  epd_poweron();
  displayPoweredOn = true;
  epd_clear();
  memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
  drawCurrentScreen();
  edp_update();
}

// Power off display
void displayPowerOff() {
  if (displayPoweredOn) {
    epd_poweroff_all();
    displayPoweredOn = false;
  }
}

// Deep clean display - multiple refresh cycles to remove ghosting
void deepCleanDisplay() {
  Serial.println("Starting deep clean...");
  epd_poweron();
  displayPoweredOn = true;

  // Do 3 cycles of black-white to clean ghosting
  for (int i = 0; i < 3; i++) {
    Serial.printf("Clean cycle %d/3\n", i + 1);
    memset(framebuffer, 0x00, EPD_WIDTH * EPD_HEIGHT / 2);  // All black
    epd_draw_grayscale_image(epd_full_screen(), framebuffer);
    delay(300);
    memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);  // All white
    epd_draw_grayscale_image(epd_full_screen(), framebuffer);
    delay(300);
  }

  epd_clear();  // Final clear
  Serial.println("Deep clean complete");

  // Redraw current screen
  refreshScreenFast();
}

// Calendar long-press tracking (hidden feature)
unsigned long calendarTouchStart = 0;
bool calendarTouchPending = false;

// Handle touch input and screen navigation
void handleTouchNavigation() {
  if (pollTouch()) {
    lastTouchTime = millis();  // Reset timeout on any touch
    ScreenState newScreen = checkTouchZone(lastTouch.x, lastTouch.y, currentScreen);

    Serial.printf("Current screen: %d, New screen: %d\n", currentScreen, newScreen);

    // Check for special deep clean action
    if (newScreen == SCREEN_CLEAN_ACTION) {
      deepCleanDisplay();
      calendarTouchPending = false;
      return;
    }

    // Check for history toggle action
    if (newScreen == SCREEN_HISTORY_TOGGLE) {
      historyShowWeek = !historyShowWeek;
      Serial.printf("History view toggled: %s\n", historyShowWeek ? "1 week" : "48 hours");
      refreshScreen();  // Full refresh to clear previous content
      calendarTouchPending = false;
      return;
    }

    // Check for calendar navigation actions
    if (newScreen == SCREEN_CALENDAR_PREV) {
      calendarMonthOffset--;
      Serial.printf("Calendar: previous month (offset: %d)\n", calendarMonthOffset);
      refreshScreen();
      return;
    }
    if (newScreen == SCREEN_CALENDAR_NEXT) {
      calendarMonthOffset++;
      Serial.printf("Calendar: next month (offset: %d)\n", calendarMonthOffset);
      refreshScreen();
      return;
    }

    // Check for calendar year navigation actions
    if (newScreen == SCREEN_CALENDAR_YEAR_PREV) {
      calendarYearOffset--;
      Serial.printf("Calendar: previous year (offset: %d)\n", calendarYearOffset);
      refreshScreen();
      return;
    }
    if (newScreen == SCREEN_CALENDAR_YEAR_NEXT) {
      calendarYearOffset++;
      Serial.printf("Calendar: next year (offset: %d)\n", calendarYearOffset);
      refreshScreen();
      return;
    }

    // Reset year offset when entering year calendar from monthly
    if (newScreen == SCREEN_CALENDAR_YEAR && currentScreen == SCREEN_CALENDAR) {
      calendarYearOffset = 0;
    }

    // Calendar requires long press (hidden feature)
    if (newScreen == SCREEN_CALENDAR && currentScreen == SCREEN_MAIN) {
      if (!calendarTouchPending) {
        calendarTouchPending = true;
        calendarTouchStart = millis();
        Serial.println("Calendar touch started - hold to activate...");
        return;  // Don't navigate yet
      } else if (millis() - calendarTouchStart >= CALENDAR_HOLD_MS) {
        Serial.println("Calendar activated after long press!");
        calendarTouchPending = false;
        calendarMonthOffset = 0;  // Reset to current month
        // Continue to normal navigation below
      } else {
        return;  // Still waiting for hold time
      }
    } else {
      calendarTouchPending = false;  // Reset if touch moves elsewhere
    }

    if (newScreen != currentScreen) {
      Serial.println("Changing screen...");
      currentScreen = newScreen;

      if (currentScreen != SCREEN_MAIN) {
        navigationModeActive = true;
        navigationStartTime = millis();
      } else {
        navigationModeActive = false;
      }

      // Fetch new quote when entering quote screen
      if (currentScreen == SCREEN_QUOTE) {
        fetchQuote();
      }

      refreshScreen();  // Full refresh for navigation
    }
  }

  // Check for timeout (configurable seconds without touch)
  unsigned long timeoutMs = config.sleep_timeout * 1000UL;
  if (millis() - lastTouchTime > timeoutMs) {
    // Don't sleep while web config is being accessed
    if (isWebActive()) {
      Serial.println("Web config active - postponing sleep");
      lastTouchTime = millis();  // Reset timeout
      return;
    }
    Serial.println("Timeout - going to sleep");
    if (!config.keep_screen_on_sleep) {
      currentScreen = SCREEN_MAIN;
    }
    navigationModeActive = false;
    refreshScreen();  // Full refresh before sleep
    displayPowerOff();
    delay(100);
    BeginSleep();
  }
}

void loop() {
  // Handle OTA updates from Arduino IDE
  ArduinoOTA.handle();

  // Handle AP mode web server
  if (inAPMode) {
    handleAPMode();

    // Check if timeout has passed to retry WiFi connection (skip if FORCE_AP_MODE)
    // Only timeout in recovery mode (not initial setup or forced mode)
    if (apModeHasTimeout() && millis() - apModeStartTime >= AP_RETRY_TIMEOUT) {
      Serial.println("AP mode timeout - retrying WiFi connection...");
      stopAPMode();
      inAPMode = false;
      currentAPModeType = AP_NONE;

      // Try to connect to WiFi
      if (StartWiFi() == WL_CONNECTED && SetupTime() == true) {
        Serial.println("WiFi connected! Fetching weather data...");
        apRetryCount = 0;  // Reset retry counter on successful connection
        // WeatherAPI: single HTTPS call gets everything (current, forecast, AQI)
        WiFiClientSecure client;
        bool RxWeather = false;
        byte Attempts = 1;
        while (RxWeather == false && Attempts <= 2) {
          RxWeather = obtainWeatherData(client);
          Attempts++;
        }
        if (RxWeather) {
          currentScreen = SCREEN_MAIN;
          epd_poweron();
          displayPoweredOn = true;
          epd_clear();
          memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
          DisplayWeather();
          edp_update();
          // Keep display on for navigation
          lastTouchTime = millis();
          navigationModeActive = true;
          return;
        }
      }

      // WiFi failed again
      apRetryCount++;
      Serial.println("WiFi retry failed (attempt " + String(apRetryCount) + "/" + String(AP_MAX_RETRIES) + ")");

      // Check if max retries reached
      if (apRetryCount >= AP_MAX_RETRIES) {
        Serial.println("Max retries reached - entering permanent deep sleep");
        Serial.println("Manual reset required to wake up");

        // Show sleep message on display
        epd_poweron();
        epd_clear();
        memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
        setFont(OpenSans12B);
        drawString(480, 200, TXT_AP_WIFI_FAILED, CENTER);
        drawString(480, 240, TXT_AP_ATTEMPTS_EXHAUSTED + " (" + String(AP_MAX_RETRIES) + ")", CENTER);
        drawString(480, 300, TXT_AP_PERMANENT_SLEEP, CENTER);
        drawString(480, 340, TXT_AP_PRESS_RESET, CENTER);
        edp_update();
        delay(2000);
        epd_poweroff();

        // Enter permanent deep sleep (no wake timer)
        esp_deep_sleep_start();
      }

      // Return to AP mode for another attempt (recovery mode with timeout)
      currentAPModeType = AP_RECOVERY;
      inAPMode = true;
      apModeStartTime = millis();
      displayAPModeScreen();
      startAPMode();
    }
    return;
  }

  // Handle web server requests for configuration
  handleWebServer();

  // Handle touch navigation
  handleTouchNavigation();
  delay(100);  // Reduce CPU load while polling
}

// Apply stored configuration to global variables
void applyStoredConfig() {
  if (strlen(config.api_key) > 0) {
    apikey = String(config.api_key);
  }
  if (strlen(config.groq_apikey) > 0) {
    groq_apikey = String(config.groq_apikey);
  }
  if (strlen(config.city) > 0) {
    City = String(config.city);
  }
  if (strlen(config.latitude) > 0) {
    Latitude = String(config.latitude);
  }
  if (strlen(config.longitude) > 0) {
    Longitude = String(config.longitude);
  }
  if (strlen(config.language) > 0) {
    Language = String(config.language);
  }
  if (strlen(config.hemisphere) > 0) {
    Hemisphere = String(config.hemisphere);
  }
  if (strlen(config.units) > 0) {
    Units = String(config.units);
  }
  if (strlen(config.timezone) > 0) {
    Timezone = config.timezone;
  }
  if (config.gmt_offset != 0 || hasValidConfig()) {
    gmtOffset_sec = config.gmt_offset;
  }
  if (config.dst_offset != 0 || hasValidConfig()) {
    daylightOffset_sec = config.dst_offset;
  }
}

// Apply settings that can change without reboot
// Returns true if display needs refresh
bool applyImmediateSettings() {
  bool needsRefresh = false;

  // Language - can change immediately
  if (strlen(config.language) > 0 && Language != String(config.language)) {
    Language = String(config.language);
    initLanguage(config.language);
    needsRefresh = true;
  }

  // Units - can change immediately
  if (strlen(config.units) > 0 && Units != String(config.units)) {
    Units = String(config.units);
    needsRefresh = true;
  }

  // Hemisphere - can change immediately
  if (strlen(config.hemisphere) > 0 && Hemisphere != String(config.hemisphere)) {
    Hemisphere = String(config.hemisphere);
    needsRefresh = true;
  }

  // Update interval - can change immediately
  if (config.update_interval > 0) {
    SleepDuration = config.update_interval;
  }

  // Wake/Sleep hours - can change immediately
  WakeupHour = config.wakeup_hour;
  SleepHour = config.sleep_hour;

  // Narrative style - can change immediately (affects next fetch)
  // No action needed, config.narrative_style is used directly

  return needsRefresh;
}

// Display AP mode info on screen
void displayAPModeScreen() {
  epd_poweron();
  epd_clear();
  memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);

  // Show mode-specific title
  setFont(OpenSans24B);
  if (currentAPModeType == AP_INITIAL_SETUP) {
    drawString(SCREEN_WIDTH / 2, 65, TXT_AP_INITIAL_SETUP, CENTER);
    setFont(OpenSans12B);
    drawString(SCREEN_WIDTH / 2, 105, TXT_AP_WIFI_CONFIG_MODE, CENTER);
  } else if (currentAPModeType == AP_RECOVERY) {
    drawString(SCREEN_WIDTH / 2, 80, TXT_AP_RECOVERY_MODE, CENTER);
    setFont(OpenSans12B);
    drawString(SCREEN_WIDTH / 2, 120, TXT_AP_WIFI_FAILED, CENTER);
  } else {
    drawString(SCREEN_WIDTH / 2, 100, TXT_AP_WIFI_CONFIG_MODE, CENTER);
  }

  setFont(OpenSans12B);
  drawString(SCREEN_WIDTH / 2, 180, TXT_AP_CONNECT_TO_WIFI, CENTER);

  setFont(OpenSans18B);
  drawString(SCREEN_WIDTH / 2, 220, AP_SSID, CENTER);

  setFont(OpenSans12B);
  drawString(SCREEN_WIDTH / 2, 270, TXT_AP_PASSWORD + " " + String(AP_PASSWORD), CENTER);

  drawString(SCREEN_WIDTH / 2, 330, TXT_AP_OPEN_BROWSER, CENTER);

  setFont(OpenSans18B);
  drawString(SCREEN_WIDTH / 2, 370, "http://192.168.4.1", CENTER);

  setFont(OpenSans10B);
  drawString(SCREEN_WIDTH / 2, 430, TXT_AP_DEVICE_RESTART, CENTER);

  // Show timeout info based on mode
  if (apModeHasTimeout()) {
    drawString(SCREEN_WIDTH / 2, 460, TXT_AP_TIMEOUT_INFO, CENTER);
    if (apRetryCount > 0) {
      drawString(SCREEN_WIDTH / 2, 490, TXT_AP_ATTEMPTS_REMAINING + " " + String(AP_MAX_RETRIES - apRetryCount), CENTER);
    }
  } else {
    drawString(SCREEN_WIDTH / 2, 460, TXT_AP_NO_TIMEOUT, CENTER);
  }

  edp_update();
  epd_poweroff_all();
}

void setup() {
  InitialiseSystem();

  // Check wake-up cause (for debugging)
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Wakeup caused by TIMER");
      break;
    default:
      Serial.printf("Wakeup was not caused by deep sleep: %d (power on or reset)\n", wakeup_reason);
      break;
  }

  // Check if BOOT button is pressed for BLE config mode
  // DISABLED - GPIO 0 reading unreliable, need different approach
  /*
  if (isBootButtonPressed()) {
    Serial.println("BOOT button pressed - entering BLE config mode...");
    epd_poweron();
    epd_clear();
    delay(100);
    // Show screen first, then init BLE
    DisplayBLEConfigScreen();
    delay(500);
    initBLE();
    // Stay in BLE mode until restart
    while (bleConfigMode) {
      // Update display if connection state changes
      if (bleDeviceConnected != bleOldDeviceConnected) {
        bleOldDeviceConnected = bleDeviceConnected;
        DisplayBLEConfigScreen();
      }
      delay(100);
    }
    return;
  }
  */

  // Initialize touch controller
  if (InitializeTouch()) {
    Serial.println("Touch controller initialized successfully");
  } else {
    Serial.println("Warning: Touch controller not available");
  }

  // Initialize SD card
  sdCardAvailable = initSDCard();

  // Initialize weather history (SD if available, FFat as fallback)
  initWeatherHistory();

  // Load stored configuration
  loadConfig();
  applyStoredConfig();

  // Load last screen from NVS (if configured to keep screen)
  if (config.keep_screen_on_sleep) {
    preferences.begin("weather", true);
    currentScreen = (ScreenState)preferences.getInt("lastscreen", SCREEN_MAIN);
    preferences.end();
  }

  // Sync sleep duration with config
  SleepDuration = config.update_interval;
  WakeupHour = config.wakeup_hour;
  SleepHour = config.sleep_hour;

  // Initialize language based on config
  initLanguage(config.language);

  // Check for first boot (no valid configuration)
  if (isFirstBoot()) {
    Serial.println("First boot detected - entering initial setup mode...");
    Serial.println("No valid WiFi or API key configured.");
    currentAPModeType = AP_INITIAL_SETUP;
    inAPMode = true;
    apRetryCount = 0;
    apModeStartTime = millis();
    displayAPModeScreen();
    startAPMode();
    return;  // Stay in loop() for AP mode handling
  }

  // Check for forced AP mode (hardcoded flag or stored preference)
  if (FORCE_AP_MODE || forceAPMode) {
    Serial.println("Entering AP mode (forced via flag)...");
    currentAPModeType = AP_FORCED;
    inAPMode = true;
    apRetryCount = 0;
    apModeStartTime = millis();
    displayAPModeScreen();
    startAPMode();
    return;  // Stay in loop() for AP mode handling
  }

  // Try to connect to WiFi
  if (StartWiFi() == WL_CONNECTED && SetupTime() == true) {
    bool WakeUp = false;
    if (WakeupHour > SleepHour)
      WakeUp = (CurrentHour >= WakeupHour || CurrentHour < SleepHour);
    else
      WakeUp = (CurrentHour >= WakeupHour && CurrentHour < SleepHour);
    if (WakeUp) {
      // WeatherAPI: single HTTPS call gets everything (current, forecast, AQI)
      byte Attempts = 1;
      bool RxWeather = false;
      WiFiClientSecure client;   // wifi client object (HTTPS)
      while (RxWeather == false && Attempts <= 2) { // Try up-to 2 times
        RxWeather = obtainWeatherData(client);
        Attempts++;
      }
      Serial.println("Received all weather data...");
      if (RxWeather) { // Only if received weather data proceed
        // Save last successful update time
        strncpy(LastUpdateTime, Time_str.c_str(), sizeof(LastUpdateTime) - 1);
        strncpy(LastUpdateDate, Date_str.c_str(), sizeof(LastUpdateDate) - 1);
        hasValidWeatherData = true;
        Serial.printf("Weather updated at: %s %s\n", LastUpdateDate, LastUpdateTime);

        // Save current weather to history
        addWeatherReading(
          WxConditions[0].Temperature,
          WxConditions[0].Humidity,
          WxConditions[0].Pressure,
          WxConditions[0].Rainfall,
          WxConditions[0].Feelslike
        );

        // Start OTA and web server for configuration (WiFi stays on)
        setupArduinoOTA();
        startWebServer();

        // Only reset to main screen if not configured to keep current screen
        if (!config.keep_screen_on_sleep) {
          currentScreen = SCREEN_MAIN;
        }
        refreshScreen();    // Display the appropriate screen based on currentScreen
        // Keep display on for touch navigation

        // Initialize touch time and let loop() handle navigation
        Serial.println("Touch navigation active - 30 second timeout");
        Serial.print("Config page: http://");
        Serial.println(WiFi.localIP());
        lastTouchTime = millis();
        navigationModeActive = true;  // Enable touch handling in loop()
        return;  // Go to loop() for continuous touch handling
      }
    }
  } else {
    // WiFi connection failed - enter AP mode (recovery mode with timeout)
    Serial.println("WiFi connection failed! Entering AP mode...");
    currentAPModeType = AP_RECOVERY;
    inAPMode = true;
    apRetryCount = 0;
    apModeStartTime = millis();
    displayAPModeScreen();
    startAPMode();
    return;  // Stay in loop() for AP mode handling
  }

  BeginSleep();
}

void Convert_Readings_to_Imperial() {
  // Pressure stays in mb/hPa for both unit systems (no conversion needed)
  WxForecast[0].Rainfall = mm_to_inches(WxForecast[0].Rainfall);
  WxConditions[0].DailyRainfall = mm_to_inches(WxConditions[0].DailyRainfall);
}

// Decode WeatherAPI JSON response (all data in one response)
bool DecodeWeatherAPI(const String& json) {
  Serial.print(F("\nCreating object...and "));
  DynamicJsonDocument doc(64 * 1024);  // WeatherAPI response is large (uses PSRAM on ESP32S3)
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return false;
  }

  JsonObject root = doc.as<JsonObject>();
  Serial.println(" Decoding WeatherAPI data");

  // Location data
  JsonObject location = root["location"];
  WxConditions[0].Region = location["region"].as<String>();
  WxConditions[0].Country = location["country"].as<String>();
  WxConditions[0].lat = location["lat"].as<float>();
  WxConditions[0].lon = location["lon"].as<float>();
  // Store station info for update info screen
  String stationFullName = location["name"].as<String>() + ", " + WxConditions[0].Region;
  strncpy(StationName, stationFullName.c_str(), sizeof(StationName) - 1);
  StationLat = WxConditions[0].lat;
  StationLon = WxConditions[0].lon;
  StationDataEpoch = location["localtime_epoch"].as<uint32_t>();
  Serial.println("Location: " + location["name"].as<String>() + ", " + WxConditions[0].Region);
  Serial.println("API Timezone: " + location["tz_id"].as<String>());
  String apiLocaltime = location["localtime"].as<String>();
  Serial.println("API Localtime: " + apiLocaltime);
  Serial.printf("Device Time: %02d:%02d:%02d (TZ: %s, GMT: %d, DST: %d)\n",
    CurrentHour, CurrentMin, CurrentSec, Timezone, gmtOffset_sec, daylightOffset_sec);

  // Check if API timezone likely has DST but device doesn't
  // WeatherAPI uses IANA timezones which include DST rules
  // Mexico abolished DST in 2022, but WeatherAPI's database may still apply it
  String apiTzId = location["tz_id"].as<String>();
  bool apiMayHaveDST = (apiTzId.indexOf("Mexico") >= 0 || apiTzId.indexOf("America/") >= 0);
  bool deviceHasNoDST = (daylightOffset_sec == 0);

  // Check if we're in DST months (roughly April-October for North America)
  // Extract month from API localtime string (format: "2026-04-11 17:36")
  int apiMonth = apiLocaltime.substring(5, 7).toInt();  // Extract "04" from position 5-6
  bool inDSTMonths = (apiMonth >= 4 && apiMonth <= 10);  // April (4) to October (10)

  // If API likely applies DST, device doesn't, and we're in DST months,
  // the astronomy data (sunrise/sunset) will be 1 hour ahead
  if (apiMayHaveDST && deviceHasNoDST && inDSTMonths) {
    // Check if the hours actually match (meaning API's "current" time doesn't show DST offset)
    // but the astronomy calculations still use DST
    String apiTimeStr = apiLocaltime.substring(apiLocaltime.indexOf(' ') + 1);
    int apiHour = apiTimeStr.substring(0, apiTimeStr.indexOf(':')).toInt();
    int hourDiff = apiHour - CurrentHour;
    if (hourDiff > 12) hourDiff -= 24;
    if (hourDiff < -12) hourDiff += 24;

    // If current times match but we're in DST months, astronomy data likely has DST applied
    if (abs(hourDiff) <= 1) {
      apiTimezoneOffset = 1;  // Astronomy data is 1 hour ahead due to DST
      Serial.println("DST correction: API astronomy data likely includes DST, device doesn't");
      Serial.printf("Applying -1 hour offset to sunrise/sunset (apiTz: %s, DST months: yes)\n", apiTzId.c_str());
    }
  } else {
    // Fallback to original hour-based calculation
    int spacePos = apiLocaltime.indexOf(' ');
    if (spacePos > 0) {
      String apiTimeStr = apiLocaltime.substring(spacePos + 1);
      int colonPos = apiTimeStr.indexOf(':');
      if (colonPos > 0) {
        int apiHour = apiTimeStr.substring(0, colonPos).toInt();
        apiTimezoneOffset = apiHour - CurrentHour;
        if (apiTimezoneOffset > 12) apiTimezoneOffset -= 24;
        if (apiTimezoneOffset < -12) apiTimezoneOffset += 24;
        if (apiTimezoneOffset != 0) {
          Serial.printf("Timezone offset detected: %d hour(s) - will adjust sunrise/sunset\n", apiTimezoneOffset);
        }
      }
    }
  }

  // Current conditions
  JsonObject current = root["current"];
  WxConditions[0].Temperature = current["temp_c"].as<float>();
  WxConditions[0].Feelslike = current["feelslike_c"].as<float>();
  WxConditions[0].Humidity = current["humidity"].as<float>();
  WxConditions[0].Pressure = current["pressure_mb"].as<float>();
  WxConditions[0].Windspeed = current["wind_kph"].as<float>();
  WxConditions[0].Winddir = current["wind_degree"].as<float>();
  WxConditions[0].WinddirStr = current["wind_dir"].as<String>();
  WxConditions[0].Gust = current["gust_kph"].as<float>();
  WxConditions[0].Cloudcover = current["cloud"].as<int>();
  WxConditions[0].Visibility = current["vis_km"].as<int>() * 1000;  // Convert to meters
  WxConditions[0].UVIndex = current["uv"].as<float>();
  WxConditions[0].Dewpoint = current["dewpoint_c"].as<float>();
  WxConditions[0].Rainfall = current["precip_mm"].as<float>();

  // Condition text and icon code
  WxConditions[0].Forecast0 = current["condition"]["text"].as<String>();
  int conditionCode = current["condition"]["code"].as<int>();
  WxConditions[0].ConditionCode = conditionCode;  // Store for short description
  WxConditions[0].Icon = mapWeatherAPIIcon(conditionCode, current["is_day"].as<int>());

  Serial.printf("Temp: %.1f, Feels: %.1f, Hum: %.0f%%\n",
    WxConditions[0].Temperature, WxConditions[0].Feelslike, WxConditions[0].Humidity);
  Serial.printf("Wind: %.1f kph %s, Gust: %.1f\n",
    WxConditions[0].Windspeed, WxConditions[0].WinddirStr.c_str(), WxConditions[0].Gust);
  Serial.printf("UV: %.1f, Condition: %s\n", WxConditions[0].UVIndex, WxConditions[0].Forecast0.c_str());

  // Air Quality (included in current)
  if (current.containsKey("air_quality")) {
    JsonObject aqi = current["air_quality"];
    WxConditions[0].CO = aqi["co"].as<float>();
    WxConditions[0].NO2 = aqi["no2"].as<float>();
    WxConditions[0].O3 = aqi["o3"].as<float>();
    WxConditions[0].SO2 = aqi["so2"].as<float>();
    WxConditions[0].PM2_5 = aqi["pm2_5"].as<float>();
    WxConditions[0].PM10 = aqi["pm10"].as<float>();
    WxConditions[0].AQI = aqi["us-epa-index"].as<int>();
    WxConditions[0].AQI_DEFRA = aqi["gb-defra-index"].as<int>();
    Serial.printf("AQI: %d, PM2.5: %.1f, PM10: %.1f\n",
      WxConditions[0].AQI, WxConditions[0].PM2_5, WxConditions[0].PM10);
  }

  // Forecast data (3 days)
  JsonArray forecastDays = root["forecast"]["forecastday"];
  int forecastIdx = 0;

  for (int day = 0; day < forecastDays.size() && day < 3; day++) {
    JsonObject dayData = forecastDays[day];
    JsonObject dayInfo = dayData["day"];
    JsonObject astro = dayData["astro"];
    JsonArray hours = dayData["hour"];

    // Store astronomy data for day 0 (today)
    if (day == 0) {
      // Get raw values from API
      String rawSunrise = astro["sunrise"].as<String>();
      String rawSunset = astro["sunset"].as<String>();
      String rawMoonrise = astro["moonrise"].as<String>();
      String rawMoonset = astro["moonset"].as<String>();

      // Apply timezone offset correction if needed (e.g., API uses DST but location doesn't)
      WxConditions[0].SunriseStr = adjustTimeByOffset(rawSunrise, apiTimezoneOffset);
      WxConditions[0].SunsetStr = adjustTimeByOffset(rawSunset, apiTimezoneOffset);
      WxConditions[0].Moonrise = adjustTimeByOffset(rawMoonrise, apiTimezoneOffset);
      WxConditions[0].Moonset = adjustTimeByOffset(rawMoonset, apiTimezoneOffset);
      WxConditions[0].MoonPhase = astro["moon_phase"].as<String>();
      WxConditions[0].MoonIllum = astro["moon_illumination"].as<int>();

      // Also store daily high/low and total precipitation for today
      WxConditions[0].High = dayInfo["maxtemp_c"].as<float>();
      WxConditions[0].Low = dayInfo["mintemp_c"].as<float>();
      WxConditions[0].DailyRainfall = dayInfo["totalprecip_mm"].as<float>();
      Serial.printf("Daily rainfall: %.1f mm\n", WxConditions[0].DailyRainfall);

      Serial.printf("Sun (raw): %s - %s\n", rawSunrise.c_str(), rawSunset.c_str());
      Serial.printf("Sun (adjusted): %s - %s\n", WxConditions[0].SunriseStr.c_str(), WxConditions[0].SunsetStr.c_str());
      Serial.printf("Moon: %s (%d%%), Rise: %s, Set: %s\n",
        WxConditions[0].MoonPhase.c_str(), WxConditions[0].MoonIllum,
        WxConditions[0].Moonrise.c_str(), WxConditions[0].Moonset.c_str());
    }

    // Process hourly data (24 hours per day, but we'll sample every 3 hours like OWM)
    for (int h = 0; h < hours.size() && forecastIdx < max_readings; h += 3) {
      JsonObject hour = hours[h];
      WxForecast[forecastIdx].Dt = hour["time_epoch"].as<int>();
      WxForecast[forecastIdx].Temperature = hour["temp_c"].as<float>();
      WxForecast[forecastIdx].Feelslike = hour["feelslike_c"].as<float>();
      WxForecast[forecastIdx].Humidity = hour["humidity"].as<float>();
      WxForecast[forecastIdx].Pressure = hour["pressure_mb"].as<float>();
      WxForecast[forecastIdx].Windspeed = hour["wind_kph"].as<float>();
      WxForecast[forecastIdx].Winddir = hour["wind_degree"].as<float>();
      WxForecast[forecastIdx].Rainfall = hour["precip_mm"].as<float>();
      WxForecast[forecastIdx].ChanceOfRain = hour["chance_of_rain"].as<int>();
      WxForecast[forecastIdx].ChanceOfSnow = hour["chance_of_snow"].as<int>();
      WxForecast[forecastIdx].Pop = hour["chance_of_rain"].as<float>() / 100.0;  // Convert to 0-1 scale
      WxForecast[forecastIdx].Cloudcover = hour["cloud"].as<int>();
      WxForecast[forecastIdx].Period = hour["time"].as<String>();

      // Map icon
      int code = hour["condition"]["code"].as<int>();
      WxForecast[forecastIdx].Icon = mapWeatherAPIIcon(code, hour["is_day"].as<int>());
      WxForecast[forecastIdx].Forecast0 = hour["condition"]["text"].as<String>();

      // Store daily high/low
      WxForecast[forecastIdx].High = dayInfo["maxtemp_c"].as<float>();
      WxForecast[forecastIdx].Low = dayInfo["mintemp_c"].as<float>();

      forecastIdx++;
    }
  }

  // Calculate pressure trend
  if (forecastIdx > 2) {
    float pressure_trend = WxForecast[0].Pressure - WxForecast[2].Pressure;
    pressure_trend = ((int)(pressure_trend * 10)) / 10.0;
    WxConditions[0].Trend = "=";
    if (pressure_trend > 0)  WxConditions[0].Trend = "+";
    if (pressure_trend < 0)  WxConditions[0].Trend = "-";
    if (pressure_trend == 0) WxConditions[0].Trend = "0";
  }

  if (Units == "I") Convert_Readings_to_Imperial();

  Serial.printf("Parsed %d forecast periods\n", forecastIdx);
  return true;
}

// Map WeatherAPI condition codes to OWM-style icon codes
String mapWeatherAPIIcon(int code, int isDay) {
  // WeatherAPI codes: https://www.weatherapi.com/docs/weather_conditions.json
  // Map to OWM icon format: 01d, 02d, 03d, 04d, 09d, 10d, 11d, 13d, 50d (d=day, n=night)
  String suffix = isDay ? "d" : "n";

  switch(code) {
    case 1000: return "01" + suffix;  // Sunny/Clear
    case 1003: return "02" + suffix;  // Partly cloudy
    case 1006: return "03" + suffix;  // Cloudy
    case 1009: return "04" + suffix;  // Overcast
    case 1030: case 1135: case 1147: return "50" + suffix;  // Mist/Fog
    case 1063: case 1180: case 1183: case 1186: case 1189: return "10" + suffix;  // Rain
    case 1066: case 1210: case 1213: case 1216: case 1219: case 1222: case 1225: return "13" + suffix;  // Snow
    case 1087: case 1273: case 1276: case 1279: case 1282: return "11" + suffix;  // Thunder
    case 1150: case 1153: case 1168: case 1171: return "09" + suffix;  // Drizzle
    case 1192: case 1195: case 1198: case 1201: return "10" + suffix;  // Heavy rain
    case 1204: case 1207: case 1237: case 1249: case 1252: return "13" + suffix;  // Sleet
    case 1114: case 1117: return "13" + suffix;  // Blizzard
    case 1240: case 1243: case 1246: return "09" + suffix;  // Showers
    case 1255: case 1258: case 1261: case 1264: return "13" + suffix;  // Snow showers
    default: return "03" + suffix;  // Default to cloudy
  }
}

String ConvertUnixTime(int unix_time) {
  // Returns either '21:12  ' or ' 09:12pm' depending on Units mode
  time_t tm = unix_time;
  struct tm *now_tm = localtime(&tm);
  char output[40];
  if (Units == "M") {
    strftime(output, sizeof(output), "%H:%M %d/%m/%y", now_tm);
  }
  else {
    strftime(output, sizeof(output), "%I:%M%P %m/%d/%y", now_tm);
  }
  return output;
}

// Fetch all weather data from WeatherAPI (current + forecast + AQI in one call)
bool obtainWeatherData(WiFiClientSecure & client) {
  client.stop();
  HTTPClient http;

  // Determine location query: use Location ID if configured, otherwise coordinates
  String locationQuery;
  if (strlen(config.location_id) > 0) {
    locationQuery = "id:" + String(config.location_id);
    Serial.println("Using Location ID: " + locationQuery);
  } else {
    locationQuery = Latitude + "," + Longitude;
    Serial.println("Using coordinates: " + locationQuery);
  }

  // WeatherAPI endpoint - all data in one call
  String uri = "/v1/forecast.json?key=" + apikey +
               "&q=" + locationQuery +
               "&days=3&aqi=yes&alerts=no&lang=" + Language;

  Serial.println("Fetching: https://" + String(server) + uri);

  // WeatherAPI requires HTTPS
  client.setInsecure();  // Skip certificate verification (for simplicity)
  http.begin(client, server, 443, uri, true);  // port 443, HTTPS = true
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    // Read response as String (more reliable with HTTPS)
    String payload = http.getString();
    Serial.printf("Response size: %d bytes\n", payload.length());

    if (!DecodeWeatherAPI(payload)) {
      client.stop();
      http.end();
      return false;
    }
    client.stop();
    http.end();
    return true;
  } else {
    Serial.printf("Connection failed, error: %s (code: %d)\n", http.errorToString(httpCode).c_str(), httpCode);
    client.stop();
    http.end();

    // If Location ID was used and failed, try fallback to coordinates
    if (strlen(config.location_id) > 0) {
      Serial.println("Location ID failed, trying coordinates fallback...");
      String fallbackUri = "/v1/forecast.json?key=" + apikey +
                   "&q=" + Latitude + "," + Longitude +
                   "&days=3&aqi=yes&alerts=no&lang=" + Language;
      Serial.println("Fetching: https://" + String(server) + fallbackUri);

      client.setInsecure();
      http.begin(client, server, 443, fallbackUri, true);
      httpCode = http.GET();

      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.printf("Fallback response size: %d bytes\n", payload.length());
        if (DecodeWeatherAPI(payload)) {
          client.stop();
          http.end();
          return true;
        }
      }
      client.stop();
      http.end();
    }
    return false;
  }
}

float mm_to_inches(float value_mm) {
  return MM_TO_INCHES * value_mm;
}

float hPa_to_inHg(float value_hPa) {
  return HPA_TO_INHG * value_hPa;
}

int JulianDate(int d, int m, int y) {
  int mm, yy, k1, k2, k3, j;
  yy = y - (int)((12 - m) / 10);
  mm = m + 9;
  if (mm >= 12) mm = mm - 12;
  k1 = (int)(365.25 * (yy + 4712));
  k2 = (int)(30.6001 * mm + 0.5);
  k3 = (int)((int)((yy / 100) + 49) * 0.75) - 38;
  // 'j' for dates in Julian calendar:
  j = k1 + k2 + d + 59 + 1;
  if (j > 2299160) j = j - k3; // 'j' is the Julian date at 12h UT (Universal Time) For Gregorian calendar:
  return j;
}

float SumOfPrecip(float DataArray[], int readings) {
  float sum = 0;
  for (int i = 0; i <= readings; i++) {
    sum += DataArray[i];
  }
  return sum;
}

String TitleCase(String text) {
  if (text.length() > 0) {
    String temp_text = text.substring(0, 1);
    temp_text.toUpperCase();
    return temp_text + text.substring(1); // Title-case the string
  }
  else return text;
}

void DisplayWeather() {                          // 4.7" e-paper display is 960x540 resolution
  DisplayStatusSection(600, 33, wifi_signal);    // Wi-Fi signal strength and Battery voltage
  DisplayGeneralInfoSection();                   // Top line of the display
  DisplayDisplayWindSection(147, 160, WxConditions[0].Winddir, WxConditions[0].Windspeed, 100);
  DisplayAstronomySection(15, 260);              // Astronomy section: Sunrise/set, Moon phase and icon
  DisplayMainWeatherSection(305, 100);           // Centre section: Location, temperature, Weather report, Wx Symbol
  DisplayWeatherIcon(840, 170);                  // Display weather icon (scale = Large)
  DisplayFeelsLike(280, 238);                    // Feels like temperature above forecast
  DisplayForecastSection(280, 240);              // 3hr forecast boxes
}

void DisplayGeneralInfoSection() {
  setFont(OpenSans14B);
  // Draw city multiple times for darker/blacker text
  drawString(20, 13, City, LEFT);
  drawString(20, 13, City, LEFT);
  drawString(20, 13, City, LEFT);
  // Date/time of last successful update (moved 40px right for more city space)
  setFont(OpenSans10B);
  drawString(340, 15, String(LastUpdateDate) + "  @ " + String(LastUpdateTime), LEFT);
}

void DisplayWeatherIcon(int x, int y) {
  DisplayConditionsSection(x, y, WxConditions[0].Icon, LargeIcon);
}

void DisplayMainWeatherSection(int x, int y) {
  setFont(OpenSans8B);
  DisplayTemperatureSection(x, y - 40);
  DisplayForecastTextSection(x - 55, y + 30);
  DisplayPressureSection(x, y + 70, WxConditions[0].Pressure, WxConditions[0].Trend);
}

void DisplayDisplayWindSection(int x, int y, float angle, float windspeed, int Cradius) {
  arrow(x, y, Cradius - 22, angle, 18, 33); // Show wind direction on outer circle of width and length
  setFont(OpenSans8B);
  int dxo, dyo, dxi, dyi;
  drawCircle(x, y, Cradius, Black);       // Draw compass circle
  drawCircle(x, y, Cradius + 1, Black);   // Draw compass circle
  drawCircle(x, y, Cradius * 0.7, Black); // Draw compass inner circle
  for (float a = 0; a < 360; a = a + 22.5) {
    dxo = Cradius * cos((a - 90) * PI / 180);
    dyo = Cradius * sin((a - 90) * PI / 180);
    if (a == 45)  drawString(dxo + x + 15, dyo + y - 18, TXT_NE, CENTER);
    if (a == 135) drawString(dxo + x + 20, dyo + y - 2,  TXT_SE, CENTER);
    if (a == 225) drawString(dxo + x - 20, dyo + y - 2,  TXT_SW, CENTER);
    if (a == 315) drawString(dxo + x - 15, dyo + y - 18, TXT_NW, CENTER);
    dxi = dxo * 0.9;
    dyi = dyo * 0.9;
    drawLine(dxo + x, dyo + y, dxi + x, dyi + y, Black);
    dxo = dxo * 0.7;
    dyo = dyo * 0.7;
    dxi = dxo * 0.9;
    dyi = dyo * 0.9;
    drawLine(dxo + x, dyo + y, dxi + x, dyi + y, Black);
  }
  drawString(x, y - Cradius - 20,     TXT_N, CENTER);
  drawString(x, y + Cradius + 10,     TXT_S, CENTER);
  drawString(x - Cradius - 15, y - 5, TXT_W, CENTER);
  drawString(x + Cradius + 10, y - 5, TXT_E, CENTER);
  drawString(x + 3, y + 45, String(angle, 0) + "°", CENTER);
  setFont(OpenSans12B);
  drawString(x, y - 55, WindDegToOrdinalDirection(angle), CENTER);
  setFont(OpenSans24B);
  drawString(x + 3, y - 23, String(windspeed, 1), CENTER);
  setFont(OpenSans12B);
  drawString(x, y + 20, (Units == "M" ? "m/s" : "mph"), CENTER);
}

String WindDegToOrdinalDirection(float winddirection) {
  if (winddirection >= 348.75 || winddirection < 11.25)  return TXT_N;
  if (winddirection >=  11.25 && winddirection < 33.75)  return TXT_NNE;
  if (winddirection >=  33.75 && winddirection < 56.25)  return TXT_NE;
  if (winddirection >=  56.25 && winddirection < 78.75)  return TXT_ENE;
  if (winddirection >=  78.75 && winddirection < 101.25) return TXT_E;
  if (winddirection >= 101.25 && winddirection < 123.75) return TXT_ESE;
  if (winddirection >= 123.75 && winddirection < 146.25) return TXT_SE;
  if (winddirection >= 146.25 && winddirection < 168.75) return TXT_SSE;
  if (winddirection >= 168.75 && winddirection < 191.25) return TXT_S;
  if (winddirection >= 191.25 && winddirection < 213.75) return TXT_SSW;
  if (winddirection >= 213.75 && winddirection < 236.25) return TXT_SW;
  if (winddirection >= 236.25 && winddirection < 258.75) return TXT_WSW;
  if (winddirection >= 258.75 && winddirection < 281.25) return TXT_W;
  if (winddirection >= 281.25 && winddirection < 303.75) return TXT_WNW;
  if (winddirection >= 303.75 && winddirection < 326.25) return TXT_NW;
  if (winddirection >= 326.25 && winddirection < 348.75) return TXT_NNW;
  return "?";
}

void DisplayTemperatureSection(int x, int y) {
  // Layout grid (all positions relative to x, y):
  //   x        x+150
  //   |        |
  // y | TEMP   | HUM%     ← Row 1: OpenSans24B
  // y+40| min|max | UV n    ← Row 2: OpenSans12B

  // Row 1: Temperature and Humidity
  setFont(OpenSans24B);
  drawString(x + 60, y, String(WxConditions[0].Temperature, 1) + "°", LEFT);
  drawString(x + 250, y, String(WxConditions[0].Humidity, 0) + "%", LEFT);

  // Row 2: Min|Max and UV Index
  setFont(OpenSans12B);
  drawString(x + 115, y + 45, String(WxConditions[0].High, 0) + "° | " + String(WxConditions[0].Low, 0) + "°", CENTER);
  drawString(x + 300, y + 50, TXT_UV_INDEX + " " + String(WxConditions[0].UVIndex, 0), CENTER);
}

void DisplayForecastTextSection(int x, int y) {
  setFont(OpenSans14B);
  // Use short description from condition code (no rainfall info added)
  String Wx_Description = getConditionShortText(WxConditions[0].ConditionCode);

  drawString(x + 28, y + 25, Wx_Description, LEFT);
}

void DisplayPressureSection(int x, int y, float pressure, String slope) {
  setFont(OpenSans14B);
  DrawPressureAndTrend(x - 25, y + 40, pressure, slope);
  if (WxConditions[0].Visibility > 0) {
    Visibility(x + 143, y + 30, String(WxConditions[0].Visibility / 1000.0, 2) + "km");
    x += 150; // Offset if visibility shown
  }
  if (WxConditions[0].Cloudcover > 0) CloudCover(x + 163, y + 30, WxConditions[0].Cloudcover);
}

void DisplayFeelsLike(int x, int y) {
  setFont(OpenSans10B);
  String feelsText = TXT_FEELS_LIKE + ": " + String(WxConditions[0].Feelslike, 1) + "°";
  drawString(x, y, feelsText, LEFT);

  // AQI to the right of feels like (10px spacing)
  String aqiDesc;
  switch(WxConditions[0].AQI) {
    case 1: aqiDesc = TXT_AQI_GOOD; break;
    case 2: aqiDesc = TXT_AQI_FAIR; break;
    case 3: aqiDesc = TXT_AQI_MODERATE; break;
    case 4: aqiDesc = TXT_AQI_POOR; break;
    case 5: aqiDesc = TXT_AQI_VERY_POOR; break;
    default: aqiDesc = "--";
  }
  String aqiText = TXT_AQI + ": " + String(WxConditions[0].AQI) + " - " + aqiDesc;
  drawString(x + 275, y - 1, aqiText, LEFT);
}

// Find first forecast index within 1 hour of current time
int getFirstRelevantForecastIndex() {
  time_t now = time(NULL);
  for (int i = 0; i < max_readings; i++) {
    long forecastLocal = WxForecast[i].Dt + WxConditions[0].Timezone;
    // If forecast local time is within 1 hour of now or in the future
    if (forecastLocal >= now - 3600) {
      return i;
    }
  }
  return 0;
}

void DisplayForecastWeather(int x, int y, int displayPos, int dataIndex) {
  int fwidth = 95;
  x = x + fwidth * displayPos;
  DisplayConditionsSection(x + fwidth / 2, y + 90, WxForecast[dataIndex].Icon, SmallIcon);
  setFont(OpenSans10B);
  drawString(x + fwidth / 2, y + 30, String(ConvertUnixTime(WxForecast[dataIndex].Dt + WxConditions[0].Timezone).substring(0, 5)), CENTER);
  drawString(x + fwidth / 2, y + 117, String(WxForecast[dataIndex].High, 0) + "°/" + String(WxForecast[dataIndex].Low, 0) + "°", CENTER);
}


double NormalizedMoonPhase(int d, int m, int y) {
  int j = JulianDate(d, m, y);
  //Calculate approximate moon phase
  double Phase = (j + 4.867) / 29.53059;
  return (Phase - (int) Phase);
}

// Translate WeatherAPI moon phase to local language
String TranslateMoonPhase(String phase) {
  phase.toLowerCase();
  phase.trim();

  if (phase.indexOf("new") >= 0)              return TXT_MOON_NEW;
  if (phase.indexOf("waxing") >= 0 && phase.indexOf("crescent") >= 0) return TXT_MOON_WAXING_CRESCENT;
  if (phase.indexOf("first") >= 0)            return TXT_MOON_FIRST_QUARTER;
  if (phase.indexOf("waxing") >= 0 && phase.indexOf("gibbous") >= 0)  return TXT_MOON_WAXING_GIBBOUS;
  if (phase.indexOf("full") >= 0)             return TXT_MOON_FULL;
  if (phase.indexOf("waning") >= 0 && phase.indexOf("gibbous") >= 0)  return TXT_MOON_WANING_GIBBOUS;
  if (phase.indexOf("last") >= 0 || phase.indexOf("third") >= 0)      return TXT_MOON_THIRD_QUARTER;
  if (phase.indexOf("waning") >= 0 && phase.indexOf("crescent") >= 0) return TXT_MOON_WANING_CRESCENT;

  return phase;
}

// Draw moon using WeatherAPI illumination data
void DrawMoonFromAPI(int x, int y, int diameter, int illumination, String phase, String hemisphere) {
  // Convert illumination (0-100%) to phase (0.0-1.0)
  // Waxing: 0 -> 0.5 (new to full), Waning: 0.5 -> 1.0 (full to new)
  double Phase;
  String phaseLower = phase;
  phaseLower.toLowerCase();

  if (phaseLower.indexOf("waxing") >= 0 || phaseLower.indexOf("first quarter") >= 0) {
    // Waxing (lit on RIGHT in northern hemisphere): Phase > 0.5
    // 5% illum → Phase 0.525 (small crescent right), 100% → Phase 1.0 (full)
    Phase = 0.5 + (illumination / 100.0) * 0.5;
  } else if (phaseLower.indexOf("waning") >= 0 || phaseLower.indexOf("last quarter") >= 0 || phaseLower.indexOf("third quarter") >= 0) {
    // Waning (lit on LEFT in northern hemisphere): Phase < 0.5
    // 100% illum → Phase 0 (full), 5% → Phase 0.475 (small crescent left)
    Phase = 0.5 - (illumination / 100.0) * 0.5;
  } else if (phaseLower.indexOf("full") >= 0) {
    Phase = 0.0;  // Full moon = all lit
  } else {
    // New moon = all dark
    Phase = 0.5;
  }

  String hemiLower = hemisphere;
  hemiLower.toLowerCase();
  if (hemiLower == "south") Phase = 1 - Phase;

  // Draw dark part of moon first
  fillCircle(x + diameter - 1, y + diameter, diameter / 2 + 1, DarkGrey);

  // Draw light part using moon drawing logic
  const int number_of_lines = 90;
  for (double Ypos = 0; Ypos <= number_of_lines / 2; Ypos++) {
    double Xpos = sqrt(number_of_lines / 2 * number_of_lines / 2 - Ypos * Ypos);
    double Rpos = 2 * Xpos;
    double Xpos1, Xpos2;
    if (Phase < 0.5) {
      Xpos1 = -Xpos;
      Xpos2 = Rpos - 2 * Phase * Rpos - Xpos;
    } else {
      Xpos1 = Xpos;
      Xpos2 = Xpos - 2 * Phase * Rpos + Rpos;
    }
    double pW1x = (Xpos1 + number_of_lines) / number_of_lines * diameter + x;
    double pW1y = (number_of_lines - Ypos) / number_of_lines * diameter + y;
    double pW2x = (Xpos2 + number_of_lines) / number_of_lines * diameter + x;
    double pW2y = (number_of_lines - Ypos) / number_of_lines * diameter + y;
    double pW3x = (Xpos1 + number_of_lines) / number_of_lines * diameter + x;
    double pW3y = (Ypos + number_of_lines) / number_of_lines * diameter + y;
    double pW4x = (Xpos2 + number_of_lines) / number_of_lines * diameter + x;
    double pW4y = (Ypos + number_of_lines) / number_of_lines * diameter + y;
    drawLine(pW1x, pW1y, pW2x, pW2y, White);
    drawLine(pW3x, pW3y, pW4x, pW4y, White);
  }
  drawCircle(x + diameter - 1, y + diameter, diameter / 2, Black);
}

// Adjust time string by offset hours (e.g., "07:22 AM" with offset -1 becomes "06:22 AM")
String adjustTimeByOffset(String timeStr, int offsetHours) {
  if (offsetHours == 0) return timeStr;

  timeStr.trim();
  timeStr.toUpperCase();

  int colonPos = timeStr.indexOf(':');
  if (colonPos < 0) return timeStr;

  int hour = timeStr.substring(0, colonPos).toInt();
  String minutePart = timeStr.substring(colonPos + 1, colonPos + 3);

  bool isPM = timeStr.indexOf("PM") >= 0;
  bool isAM = timeStr.indexOf("AM") >= 0;

  // Convert to 24-hour for calculation
  if (isPM && hour != 12) hour += 12;
  if (isAM && hour == 12) hour = 0;

  // Apply offset (subtract because API time ahead means sunrise/sunset need to be earlier)
  hour -= offsetHours;
  if (hour < 0) hour += 24;
  if (hour >= 24) hour -= 24;

  // Convert back to 12-hour format
  bool newPM = (hour >= 12);
  int hour12 = hour % 12;
  if (hour12 == 0) hour12 = 12;

  char buf[12];
  sprintf(buf, "%02d:%s %s", hour12, minutePart.c_str(), newPM ? "PM" : "AM");
  return String(buf);
}

// Convert "06:40 AM" or "06:48 PM" to 24-hour format "06:40" or "18:48"
String convertTo24Hour(String timeStr) {
  timeStr.trim();
  timeStr.toUpperCase();

  int colonPos = timeStr.indexOf(':');
  if (colonPos < 0) return timeStr;

  int hour = timeStr.substring(0, colonPos).toInt();
  String minutePart = timeStr.substring(colonPos + 1, colonPos + 3);

  bool isPM = timeStr.indexOf("PM") >= 0;
  bool isAM = timeStr.indexOf("AM") >= 0;

  if (isPM && hour != 12) hour += 12;
  if (isAM && hour == 12) hour = 0;

  char buf[6];
  sprintf(buf, "%02d:%s", hour, minutePart.c_str());
  return String(buf);
}

void DisplayAstronomySection(int x, int y) {
  setFont(OpenSans10B);

  // Moon phase text from WeatherAPI (translated)
  String moonPhaseText = TranslateMoonPhase(WxConditions[0].MoonPhase);
  drawString(x + 5, y + 95, moonPhaseText, LEFT);

  DrawMoonImage(x + 10, y + 23-5);
  // Draw moon using illumination from WeatherAPI
  DrawMoonFromAPI(x - 28, y - 15-5, 75, WxConditions[0].MoonIllum, WxConditions[0].MoonPhase, Hemisphere);

  // Sunrise/Sunset in 24-hour format
  drawString(x + 115, y + 30, convertTo24Hour(WxConditions[0].SunriseStr), LEFT);
  drawString(x + 115, y + 70, convertTo24Hour(WxConditions[0].SunsetStr), LEFT);
  DrawSunriseImage(x + 180, y + 10);
  DrawSunsetImage(x + 180, y + 50);
}

void DrawMoon(int x, int y, int diameter, int dd, int mm, int yy, String hemisphere) {
  double Phase = NormalizedMoonPhase(dd, mm, yy);
  hemisphere.toLowerCase();
  if (hemisphere == "south") Phase = 1 - Phase;
  // Draw dark part of moon
  fillCircle(x + diameter - 1, y + diameter, diameter / 2 + 1, DarkGrey);
  const int number_of_lines = 90;
  for (double Ypos = 0; Ypos <= number_of_lines / 2; Ypos++) {
    double Xpos = sqrt(number_of_lines / 2 * number_of_lines / 2 - Ypos * Ypos);
    // Determine the edges of the lighted part of the moon
    double Rpos = 2 * Xpos;
    double Xpos1, Xpos2;
    if (Phase < 0.5) {
      Xpos1 = -Xpos;
      Xpos2 = Rpos - 2 * Phase * Rpos - Xpos;
    }
    else {
      Xpos1 = Xpos;
      Xpos2 = Xpos - 2 * Phase * Rpos + Rpos;
    }
    // Draw light part of moon
    double pW1x = (Xpos1 + number_of_lines) / number_of_lines * diameter + x;
    double pW1y = (number_of_lines - Ypos)  / number_of_lines * diameter + y;
    double pW2x = (Xpos2 + number_of_lines) / number_of_lines * diameter + x;
    double pW2y = (number_of_lines - Ypos)  / number_of_lines * diameter + y;
    double pW3x = (Xpos1 + number_of_lines) / number_of_lines * diameter + x;
    double pW3y = (Ypos + number_of_lines)  / number_of_lines * diameter + y;
    double pW4x = (Xpos2 + number_of_lines) / number_of_lines * diameter + x;
    double pW4y = (Ypos + number_of_lines)  / number_of_lines * diameter + y;
    drawLine(pW1x, pW1y, pW2x, pW2y, White);
    drawLine(pW3x, pW3y, pW4x, pW4y, White);
  }
  drawCircle(x + diameter - 1, y + diameter, diameter / 2, Black);
}

String MoonPhase(int d, int m, int y, String hemisphere) {
  int c, e;
  double jd;
  int b;
  if (m < 3) {
    y--;
    m += 12;
  }
  ++m;
  c   = 365.25 * y;
  e   = 30.6  * m;
  jd  = c + e + d - 694039.09;     /* jd is total days elapsed */
  jd /= 29.53059;                        /* divide by the moon cycle (29.53 days) */
  b   = jd;                              /* int(jd) -> b, take integer part of jd */
  jd -= b;                               /* subtract integer part to leave fractional part of original jd */
  b   = jd * 8 + 0.5;                /* scale fraction from 0-8 and round by adding 0.5 */
  b   = b & 7;                           /* 0 and 8 are the same phase so modulo 8 for 0 */
  if (hemisphere == "south") b = 7 - b;
  if (b == 0) return TXT_MOON_NEW;              // New;              0%  illuminated
  if (b == 1) return TXT_MOON_WAXING_CRESCENT;  // Waxing crescent; 25%  illuminated
  if (b == 2) return TXT_MOON_FIRST_QUARTER;    // First quarter;   50%  illuminated
  if (b == 3) return TXT_MOON_WAXING_GIBBOUS;   // Waxing gibbous;  75%  illuminated
  if (b == 4) return TXT_MOON_FULL;             // Full;            100% illuminated
  if (b == 5) return TXT_MOON_WANING_GIBBOUS;   // Waning gibbous;  75%  illuminated
  if (b == 6) return TXT_MOON_THIRD_QUARTER;    // Third quarter;   50%  illuminated
  if (b == 7) return TXT_MOON_WANING_CRESCENT;  // Waning crescent; 25%  illuminated
  return "";
}

void DrawMoonImage(int x, int y) {
  Rect_t area = {
    .x = x, .y = y, .width  = moon_width, .height =  moon_height
  };
  epd_draw_grayscale_image(area, (uint8_t *) moon_data);
}

void DrawSunriseImage(int x, int y) {
  Rect_t area = {
    .x = x, .y = y, .width  = sunrise_width, .height =  sunrise_height
  };
  epd_draw_grayscale_image(area, (uint8_t *) sunrise_data);
}

void DrawSunsetImage(int x, int y) {
  Rect_t area = {
    .x = x, .y = y, .width  = sunset_width, .height =  sunset_height
  };
  epd_draw_grayscale_image(area, (uint8_t *) sunset_data);
}


void DisplayForecastSection(int x, int y) {
  const int main_screen_readings = 24;  // 3 days for main screen graphs
  int startIdx = getFirstRelevantForecastIndex();
  for(int i = 0; i < min(7, max_readings - startIdx); i++) {
    DisplayForecastWeather(x, y, i, startIdx + i);
  }
  int r = 0;
  do { // Pre-load temporary arrays with with data - because C parses by reference and remember that[1] has already been converted to I units
    if (Units == "I") pressure_readings[r] = WxForecast[r].Pressure * HPA_TO_INHG;   else pressure_readings[r] = WxForecast[r].Pressure;
    if (Units == "I") rain_readings[r]     = WxForecast[r].Rainfall * MM_TO_INCHES; else rain_readings[r]     = WxForecast[r].Rainfall;
    if (Units == "I") snow_readings[r]     = WxForecast[r].Snowfall * MM_TO_INCHES; else snow_readings[r]     = WxForecast[r].Snowfall;
    temperature_readings[r]                = WxForecast[r].Temperature;
    humidity_readings[r]                   = WxForecast[r].Humidity;
    r++;
  } while (r < max_readings);
  int gwidth = 175, gheight = 100;
  int gx = (SCREEN_WIDTH - gwidth * 4) / 5 + 8;
  int gy = (SCREEN_HEIGHT - gheight - 30);
  int gap = gwidth + gx;
  // (x,y,width,height,MinValue, MaxValue, Title, Data Array, AutoScale, ChartMode) - Using 24 readings (3 days) for main screen
  DrawGraph(gx - 10, gy, gwidth, gheight, 10, 30, Units == "M" ? TXT_TEMPERATURE_C : TXT_TEMPERATURE_F, temperature_readings, main_screen_readings, autoscale_on, barchart_off, 15);
  DrawGraph(gx + 1 * gap, gy, gwidth, gheight, 900, 1050, Units == "M" ? TXT_PRESSURE_HPA : TXT_PRESSURE_IN, pressure_readings, main_screen_readings, autoscale_on, barchart_off);
  DrawGraph(gx + 2 * gap, gy, gwidth, gheight, 0, 100,   TXT_HUMIDITY_PERCENT, humidity_readings, main_screen_readings, autoscale_off, barchart_off);
  if (SumOfPrecip(rain_readings, main_screen_readings) >= SumOfPrecip(snow_readings, main_screen_readings))
    DrawGraph(gx + 3 * gap, gy, gwidth, gheight, 0, 30, Units == "M" ? TXT_RAINFALL_MM : TXT_RAINFALL_IN, rain_readings, main_screen_readings, autoscale_on, barchart_on);
  else
    DrawGraph(gx + 3 * gap, gy, gwidth, gheight, 0, 30, Units == "M" ? TXT_SNOWFALL_MM : TXT_SNOWFALL_IN, snow_readings, main_screen_readings, autoscale_on, barchart_on);
}

void DisplayConditionsSection(int x, int y, String IconName, bool IconSize) {
  Serial.println("Icon name: " + IconName);
  if      (IconName == "01d" || IconName == "01n")  Sunny(x, y, IconSize, IconName);
  else if (IconName == "02d" || IconName == "02n")  MostlySunny(x, y, IconSize, IconName);
  else if (IconName == "03d" || IconName == "03n")  Cloudy(x, y, IconSize, IconName);
  else if (IconName == "04d" || IconName == "04n")  MostlySunny(x, y, IconSize, IconName);
  else if (IconName == "09d" || IconName == "09n")  ChanceRain(x, y, IconSize, IconName);
  else if (IconName == "10d" || IconName == "10n")  Rain(x, y, IconSize, IconName);
  else if (IconName == "11d" || IconName == "11n")  Tstorms(x, y, IconSize, IconName);
  else if (IconName == "13d" || IconName == "13n")  Snow(x, y, IconSize, IconName);
  else if (IconName == "50d")                       Haze(x, y, IconSize, IconName);
  else if (IconName == "50n")                       Fog(x, y, IconSize, IconName);
  else                                              Nodata(x, y, IconSize, IconName);
}

void arrow(int x, int y, int asize, float aangle, int pwidth, int plength) {
  float dx = (asize - 10) * cos((aangle - 90) * PI / 180) + x; // calculate X position
  float dy = (asize - 10) * sin((aangle - 90) * PI / 180) + y; // calculate Y position
  float x1 = 0;         float y1 = plength;
  float x2 = pwidth / 2;  float y2 = pwidth / 2;
  float x3 = -pwidth / 2; float y3 = pwidth / 2;
  float angle = aangle * PI / 180 - 135;
  float xx1 = x1 * cos(angle) - y1 * sin(angle) + dx;
  float yy1 = y1 * cos(angle) + x1 * sin(angle) + dy;
  float xx2 = x2 * cos(angle) - y2 * sin(angle) + dx;
  float yy2 = y2 * cos(angle) + x2 * sin(angle) + dy;
  float xx3 = x3 * cos(angle) - y3 * sin(angle) + dx;
  float yy3 = y3 * cos(angle) + x3 * sin(angle) + dy;
  fillTriangle(xx1, yy1, xx3, yy3, xx2, yy2, Black);
}

void DrawSegment(int x, int y, int o1, int o2, int o3, int o4, int o11, int o12, int o13, int o14) {
  drawLine(x + o1,  y + o2,  x + o3,  y + o4,  Black);
  drawLine(x + o11, y + o12, x + o13, y + o14, Black);
}

void DrawPressureAndTrend(int x, int y, float pressure, String slope) {
  drawString(x + 25, y - 10, String(pressure, (Units == "M" ? 0 : 1)) + (Units == "M" ? (currentLang == 0 ? "mb" : "hPa") : "in"), LEFT);
  // Arrow with shaft and head for pressure trend
  int ax = x + 8;  // Arrow center x
  int ay = y;      // Arrow center y (lowered)
  if (slope == "+") {
    // Upward arrow
    fillRect(ax - 2, ay + 2, 5, 14, Black);  // Shaft
    fillTriangle(ax, ay - 4, ax - 6, ay + 4, ax + 6, ay + 4, Black);  // Head
  }
  else if (slope == "0" || slope == "=") {
    // Rightward arrow (stable)
    fillRect(ax - 8, ay - 3, 16, 6, Black);  // Shaft
    fillTriangle(ax + 14, ay, ax + 4, ay - 8, ax + 4, ay + 8, Black);  // Head
  }
  else if (slope == "-") {
    // Downward arrow
    fillRect(ax - 2, ay - 8, 5, 14, Black);  // Shaft
    fillTriangle(ax, ay + 10, ax - 6, ay + 2, ax + 6, ay + 2, Black);  // Head
  }
}

void DisplayStatusSection(int x, int y, int rssi) {
  setFont(OpenSans10B);
  // Layout: [SD icon] [Battery icon + text] [WiFi signal]
  DrawSDCard(x + 175, y - 15);      // SD icon first (5px higher)
  DrawBattery(x + 230, y);          // Battery second (+15px)
  DrawRSSI(x + 295, y + 15, rssi);  // WiFi signal (unchanged)
}

void DrawRSSI(int x, int y, int rssi) {
  int WIFIsignal = 0;
  int xpos = 1;
  for (int _rssi = -110; _rssi <= rssi; _rssi = _rssi + 20) {
    if (_rssi <= -20)  WIFIsignal = 30; // <-20dBm displays 5-bars
    if (_rssi <= -40)  WIFIsignal = 24; // -40dBm to -21dBm displays 4-bars
    if (_rssi <= -60)  WIFIsignal = 18; // -60dBm to -41dBm displays 3-bars
    if (_rssi <= -80)  WIFIsignal = 12; // -80dBm to -61dBm displays 2-bars
    if (_rssi <= -100) WIFIsignal = 6;  // -100dBm to -81dBm displays 1-bar
    if (_rssi <= -100) WIFIsignal = 6;  // -110dBm to -101dBm displays 1-bar
    fillRect(x + xpos * 8, y - WIFIsignal, 6, WIFIsignal, Black);
    xpos++;
  }
  // Show signal strength in dBm below bars
  setFont(OpenSans8B);
  drawString(x + 25, y + 8, String(rssi) + "dB", CENTER);
}

// Draw SD card icon (only when SD is available)
void DrawSDCard(int x, int y) {
  if (!sdCardAvailable) return;

  // SD card shape: 20x26 pixels with cut corner
  // Main body
  fillRect(x, y + 6, 20, 20, Black);
  // Cut corner (top-right)
  fillRect(x + 14, y, 6, 6, Black);
  fillTriangle(x + 14, y, x + 20, y, x + 20, y + 6, White);
  // Inner lines (contact pins representation)
  for (int i = 0; i < 4; i++) {
    fillRect(x + 3 + i * 4, y + 10, 2, 10, White);
  }
  // "SD" text below
  setFont(OpenSans8B);
  drawString(x + 10, y + 28, "SD", CENTER);
}

boolean UpdateLocalTime() {
  struct tm timeinfo;
  char   time_output[30], day_output[30], update_time[30];
  while (!getLocalTime(&timeinfo, 5000)) { // Wait for 5-sec for time to synchronise
    Serial.println("Failed to obtain time");
    return false;
  }
  CurrentHour = timeinfo.tm_hour;
  CurrentMin  = timeinfo.tm_min;
  CurrentSec  = timeinfo.tm_sec;
  //See http://www.cplusplus.com/reference/ctime/strftime/
  Serial.println(&timeinfo, "%a %b %d %Y   %H:%M:%S");      // Displays: Saturday, June 24 2017 14:05:49
  if (Units == "M") {
    sprintf(day_output, "%s, %02u %s %04u", weekday_D[timeinfo.tm_wday], timeinfo.tm_mday, month_M[timeinfo.tm_mon], (timeinfo.tm_year) + 1900);
    strftime(update_time, sizeof(update_time), "%H:%M:%S", &timeinfo);  // Creates: '@ 14:05:49'   and change from 30 to 8 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    sprintf(time_output, "%s", update_time);
  }
  else
  {
    strftime(day_output, sizeof(day_output), "%a %b-%d-%Y", &timeinfo); // Creates  'Sat May-31-2019'
    strftime(update_time, sizeof(update_time), "%r", &timeinfo);        // Creates: '@ 02:05:49pm'
    sprintf(time_output, "%s", update_time);
  }
  Date_str = day_output;
  Time_str = time_output;
  return true;
}

void DrawBattery(int x, int y) {
  uint8_t percentage = 100;
  esp_adc_cal_characteristics_t adc_chars;
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
  if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
    Serial.printf("eFuse Vref:%u mV", adc_chars.vref);
    vref = adc_chars.vref;
  }
  float voltage = analogRead(14) / 4096.0 * 6.566 * (vref / 1000.0);
  Serial.println("\nVoltage = " + String(voltage));
  if (voltage > 1 ) { // Only display if there is a valid reading
    int pct = (voltage - 3.2) * 100;  // 3.2V=0%, 4.2V=100%
    if (pct > 100) pct = 100;
    if (pct < 0) pct = 0;
    percentage = pct;
    // Battery icon on top
    drawRect(x, y - 14, 42, 15, Black);           // Battery outline (42px wide)
    fillRect(x + 42, y - 10, 4, 7, Black);        // Battery tip
    fillRect(x + 2, y - 12, 38 * percentage / 100.0, 11, Black);  // Fill level
    // Percentage and voltage below icon
    setFont(OpenSans8B);
    drawString(x + 20, y + 10, String(percentage) + "% " + String(voltage, 1) + "v", CENTER);
  }
}

// Symbols are drawn on a relative 10x10grid and 1 scale unit = 1 drawing unit
void addcloud(int x, int y, int scale, int linesize) {
  fillCircle(x - scale * 3, y, scale, Black);                                                              // Left most circle
  fillCircle(x + scale * 3, y, scale, Black);                                                              // Right most circle
  fillCircle(x - scale, y - scale, scale * 1.4, Black);                                                    // left middle upper circle
  fillCircle(x + scale * 1.5, y - scale * 1.3, scale * 1.75, Black);                                       // Right middle upper circle
  fillRect(x - scale * 3 - 1, y - scale, scale * 6, scale * 2 + 1, Black);                                 // Upper and lower lines
  fillCircle(x - scale * 3, y, scale - linesize, White);                                                   // Clear left most circle
  fillCircle(x + scale * 3, y, scale - linesize, White);                                                   // Clear right most circle
  fillCircle(x - scale, y - scale, scale * 1.4 - linesize, White);                                         // left middle upper circle
  fillCircle(x + scale * 1.5, y - scale * 1.3, scale * 1.75 - linesize, White);                            // Right middle upper circle
  fillRect(x - scale * 3 + 2, y - scale + linesize - 1, scale * 5.9, scale * 2 - linesize * 2 + 2, White); // Upper and lower lines
}

void addrain(int x, int y, int scale, bool IconSize) {
  if (IconSize == SmallIcon) {
    setFont(OpenSans8B);
    drawString(x - 25, y + 12, "///////", LEFT);
  }
  else
  {
    setFont(OpenSans18B);
    drawString(x - 60, y + 25, "///////", LEFT);
  }
}

void addsnow(int x, int y, int scale, bool IconSize) {
  if (IconSize == SmallIcon) {
    setFont(OpenSans8B);
    drawString(x - 25, y + 15, "* * * *", LEFT);
  }
  else
  {
    setFont(OpenSans18B);
    drawString(x - 60, y + 30, "* * * *", LEFT);
  }
}

void addtstorm(int x, int y, int scale) {
  y = y + scale / 2;
  for (int i = 0; i < 5; i++) {
    drawLine(x - scale * 4 + scale * i * 1.5 + 0, y + scale * 1.5, x - scale * 3.5 + scale * i * 1.5 + 0, y + scale, Black);
    if (scale != Small) {
      drawLine(x - scale * 4 + scale * i * 1.5 + 1, y + scale * 1.5, x - scale * 3.5 + scale * i * 1.5 + 1, y + scale, Black);
      drawLine(x - scale * 4 + scale * i * 1.5 + 2, y + scale * 1.5, x - scale * 3.5 + scale * i * 1.5 + 2, y + scale, Black);
    }
    drawLine(x - scale * 4 + scale * i * 1.5, y + scale * 1.5 + 0, x - scale * 3 + scale * i * 1.5 + 0, y + scale * 1.5 + 0, Black);
    if (scale != Small) {
      drawLine(x - scale * 4 + scale * i * 1.5, y + scale * 1.5 + 1, x - scale * 3 + scale * i * 1.5 + 0, y + scale * 1.5 + 1, Black);
      drawLine(x - scale * 4 + scale * i * 1.5, y + scale * 1.5 + 2, x - scale * 3 + scale * i * 1.5 + 0, y + scale * 1.5 + 2, Black);
    }
    drawLine(x - scale * 3.5 + scale * i * 1.4 + 0, y + scale * 2.5, x - scale * 3 + scale * i * 1.5 + 0, y + scale * 1.5, Black);
    if (scale != Small) {
      drawLine(x - scale * 3.5 + scale * i * 1.4 + 1, y + scale * 2.5, x - scale * 3 + scale * i * 1.5 + 1, y + scale * 1.5, Black);
      drawLine(x - scale * 3.5 + scale * i * 1.4 + 2, y + scale * 2.5, x - scale * 3 + scale * i * 1.5 + 2, y + scale * 1.5, Black);
    }
  }
}

void addsun(int x, int y, int scale, bool IconSize) {
  int linesize = 5;
  fillRect(x - scale * 2, y, scale * 4, linesize, Black);
  fillRect(x, y - scale * 2, linesize, scale * 4, Black);
  drawLine(x - scale * 1.3, y - scale * 1.3, x + scale * 1.3, y + scale * 1.3, Black);
  drawLine(x - scale * 1.3, y + scale * 1.3, x + scale * 1.3, y - scale * 1.3, Black);
  if (IconSize == LargeIcon) {
    drawLine(1 + x - scale * 1.3, y - scale * 1.3, 1 + x + scale * 1.3, y + scale * 1.3, Black);
    drawLine(2 + x - scale * 1.3, y - scale * 1.3, 2 + x + scale * 1.3, y + scale * 1.3, Black);
    drawLine(3 + x - scale * 1.3, y - scale * 1.3, 3 + x + scale * 1.3, y + scale * 1.3, Black);
    drawLine(1 + x - scale * 1.3, y + scale * 1.3, 1 + x + scale * 1.3, y - scale * 1.3, Black);
    drawLine(2 + x - scale * 1.3, y + scale * 1.3, 2 + x + scale * 1.3, y - scale * 1.3, Black);
    drawLine(3 + x - scale * 1.3, y + scale * 1.3, 3 + x + scale * 1.3, y - scale * 1.3, Black);
  }
  fillCircle(x, y, scale * 1.3, White);
  fillCircle(x, y, scale, Black);
  fillCircle(x, y, scale - linesize, White);
}

void addfog(int x, int y, int scale, int linesize, bool IconSize) {
  if (IconSize == SmallIcon) {
    y -= 10;
    linesize = 1;
  }
  for (int i = 0; i < 6; i++) {
    fillRect(x - scale * 3, y + scale * 1.5, scale * 6, linesize, Black);
    fillRect(x - scale * 3, y + scale * 2.0, scale * 6, linesize, Black);
    fillRect(x - scale * 3, y + scale * 2.5, scale * 6, linesize, Black);
  }
}

void Sunny(int x, int y, bool IconSize, String IconName) {
  int scale = Small, Offset = 10;
  if (iconScaleOverride > 0) {
    scale = iconScaleOverride;
    Offset = 45;
  } else if (IconSize == LargeIcon) {
    scale = Large;
    Offset = 35;
  }
  else y = y - 3; // Shift up small sun icon
  if (IconName.endsWith("n")) addmoon(x, y + Offset, scale, IconSize);
  scale = scale * 1.6;
  addsun(x, y, scale, IconSize);
}

void MostlySunny(int x, int y, bool IconSize, String IconName) {
  int scale = Small, linesize = 5, Offset = 10;
  if (iconScaleOverride > 0) {
    scale = iconScaleOverride;
    Offset = 45;
  } else if (IconSize == LargeIcon) {
    scale = Large;
    Offset = 35;
  }
  if (IconName.endsWith("n")) addmoon(x, y + Offset, scale, IconSize);
  addsun(x - scale * 1.8, y - scale * 1.8, scale, IconSize);
  addcloud(x, y, scale, linesize);
}

void MostlyCloudy(int x, int y, bool IconSize, String IconName) {
  int scale = Small, linesize = 5, Offset = 10;
  if (iconScaleOverride > 0) {
    scale = iconScaleOverride;
    Offset = 45;
  } else if (IconSize == LargeIcon) {
    scale = Large;
    Offset = 35;
  }
  if (IconName.endsWith("n")) addmoon(x, y + Offset, scale, IconSize);
  addcloud(x, y, scale, linesize);
  addsun(x - scale * 1.8, y - scale * 1.8, scale, IconSize);
}

void Cloudy(int x, int y, bool IconSize, String IconName) {
  int scale = Small, linesize = 5, Offset = 10;
  if (iconScaleOverride > 0) {
    scale = iconScaleOverride;
    Offset = 45;
  } else if (IconSize == LargeIcon) {
    scale = Large;
    Offset = 35;
  }
  if (IconName.endsWith("n")) addmoon(x, y + Offset, scale, IconSize);
  addcloud(x + 15, y - 22, scale / 2, linesize); // Cloud top right
  addcloud(x - 10, y - 18, scale / 2, linesize); // Cloud top left
  addcloud(x, y, scale, linesize);             // Main cloud
}

void Rain(int x, int y, bool IconSize, String IconName) {
  int scale = Small, linesize = 5, Offset = 10;
  if (iconScaleOverride > 0) {
    scale = iconScaleOverride;
    Offset = 45;
  } else if (IconSize == LargeIcon) {
    scale = Large;
    Offset = 35;
  }
  if (IconName.endsWith("n")) addmoon(x, y + Offset, scale, IconSize);
  addcloud(x, y, scale, linesize);
  addrain(x, y, scale, IconSize);
}

void ExpectRain(int x, int y, bool IconSize, String IconName) {
  int scale = Small, linesize = 5, Offset = 10;
  if (iconScaleOverride > 0) {
    scale = iconScaleOverride;
    Offset = 45;
  } else if (IconSize == LargeIcon) {
    scale = Large;
    Offset = 35;
  }
  if (IconName.endsWith("n")) addmoon(x, y + Offset, scale, IconSize);
  addsun(x - scale * 1.8, y - scale * 1.8, scale, IconSize);
  addcloud(x, y, scale, linesize);
  addrain(x, y, scale, IconSize);
}

void ChanceRain(int x, int y, bool IconSize, String IconName) {
  int scale = Small, linesize = 5, Offset = 10;
  if (iconScaleOverride > 0) {
    scale = iconScaleOverride;
    Offset = 45;
  } else if (IconSize == LargeIcon) {
    scale = Large;
    Offset = 35;
  }
  if (IconName.endsWith("n")) addmoon(x, y + Offset, scale, IconSize);
  addsun(x - scale * 1.8, y - scale * 1.8, scale, IconSize);
  addcloud(x, y, scale, linesize);
  addrain(x, y, scale, IconSize);
}

void Tstorms(int x, int y, bool IconSize, String IconName) {
  int scale = Small, linesize = 5, Offset = 10;
  if (iconScaleOverride > 0) {
    scale = iconScaleOverride;
    Offset = 45;
  } else if (IconSize == LargeIcon) {
    scale = Large;
    Offset = 35;
  }
  if (IconName.endsWith("n")) addmoon(x, y + Offset, scale, IconSize);
  addcloud(x, y, scale, linesize);
  addtstorm(x, y, scale);
}

void Snow(int x, int y, bool IconSize, String IconName) {
  int scale = Small, linesize = 5, Offset = 10;
  if (iconScaleOverride > 0) {
    scale = iconScaleOverride;
    Offset = 45;
  } else if (IconSize == LargeIcon) {
    scale = Large;
    Offset = 35;
  }
  if (IconName.endsWith("n")) addmoon(x, y + Offset, scale, IconSize);
  addcloud(x, y, scale, linesize);
  addsnow(x, y, scale, IconSize);
}

void Fog(int x, int y, bool IconSize, String IconName) {
  int scale = Small, linesize = 5, Offset = 10;
  if (iconScaleOverride > 0) {
    scale = iconScaleOverride;
    Offset = 45;
  } else if (IconSize == LargeIcon) {
    scale = Large;
    Offset = 35;
  }
  if (IconName.endsWith("n")) addmoon(x, y + Offset, scale, IconSize);
  addcloud(x, y - 5, scale, linesize);
  addfog(x, y - 5, scale, linesize, IconSize);
}

void Haze(int x, int y, bool IconSize, String IconName) {
  int scale = Small, linesize = 5, Offset = 10;
  if (iconScaleOverride > 0) {
    scale = iconScaleOverride;
    Offset = 45;
  } else if (IconSize == LargeIcon) {
    scale = Large;
    Offset = 35;
  }
  if (IconName.endsWith("n")) addmoon(x, y + Offset, scale, IconSize);
  addsun(x, y - 5, scale * 1.4, IconSize);
  addfog(x, y - 5, scale * 1.4, linesize, IconSize);
}

void CloudCover(int x, int y, int CCover) {
  addcloud(x - 9, y + 2+5, Small * 0.4, 2); // Cloud top left
  addcloud(x + 3, y - 2+5, Small * 0.4, 2); // Cloud top right
  addcloud(x, y + 10+5, Small * 0.7, 2); // Main cloud
  drawString(x + 30, y, String(CCover) + "%", LEFT);
}

void Visibility(int x, int y, String Visi) {
  float start_angle = 0.52, end_angle = 2.61, Offset = 10;
  int r = 16;
  for (float i = start_angle; i < end_angle; i = i + 0.05) {
    drawPixel(x + r * cos(i), y - r / 2 + r * sin(i) + Offset, Black);
    drawPixel(x + r * cos(i), 1 + y - r / 2 + r * sin(i) + Offset, Black);
  }
  start_angle = 3.61; end_angle = 5.78;
  for (float i = start_angle; i < end_angle; i = i + 0.05) {
    drawPixel(x + r * cos(i), y + r / 2 + r * sin(i) + Offset, Black);
    drawPixel(x + r * cos(i), 1 + y + r / 2 + r * sin(i) + Offset, Black);
  }
  fillCircle(x, y + Offset, r / 4, Black);
  drawString(x + 20, y, Visi, LEFT);
}

void addmoon(int x, int y, int scale, bool IconSize) {
  if (IconSize == LargeIcon) {
    fillCircle(x - 85, y - 100, uint16_t(scale * 0.8), Black);
    fillCircle(x - 57, y - 100, uint16_t(scale * 1.6), White);
  }
  else
  {
    fillCircle(x - 28, y - 37, uint16_t(scale * 1.0), Black);
    fillCircle(x - 20, y - 37, uint16_t(scale * 1.6), White);
  }
}

void Nodata(int x, int y, bool IconSize, String IconName) {
  if (IconSize == LargeIcon) setFont(OpenSans24B); else setFont(OpenSans12B);
  drawString(x - 3, y - 10, "?", CENTER);
}

/* (C) D L BIRD
    This function will draw a graph on a ePaper/TFT/LCD display using data from an array containing data to be graphed.
    The variable 'max_readings' determines the maximum number of data elements for each array. Call it with the following parametric data:
    x_pos-the x axis top-left position of the graph
    y_pos-the y-axis top-left position of the graph, e.g. 100, 200 would draw the graph 100 pixels along and 200 pixels down from the top-left of the screen
    width-the width of the graph in pixels
    height-height of the graph in pixels
    Y1_Max-sets the scale of plotted data, for example 5000 would scale all data to a Y-axis of 5000 maximum
    data_array1 is parsed by value, externally they can be called anything else, e.g. within the routine it is called data_array1, but externally could be temperature_readings
    auto_scale-a logical value (TRUE or FALSE) that switches the Y-axis autoscale On or Off
    barchart_on-a logical value (TRUE or FALSE) that switches the drawing mode between barhcart and line graph
    barchart_colour-a sets the title and graph plotting colour
    If called with Y!_Max value of 500 and the data never goes above 500, then autoscale will retain a 0-500 Y scale, if on, the scale increases/decreases to match the data.
    auto_scale_margin, e.g. if set to 1000 then autoscale increments the scale by 1000 steps.
*/
void DrawGraph(int x_pos, int y_pos, int gwidth, int gheight, float Y1Min, float Y1Max, String title, float DataArray[], int readings, boolean auto_scale, boolean barchart_mode, int title_x_offset, int hours_span) {
#define auto_scale_margin 0 // Sets the autoscale increment, so axis steps up fter a change of e.g. 3
#define y_minor_axis 5      // 5 y-axis division markers
  setFont(OpenSans9B);
  int maxYscale = -10000;
  int minYscale =  10000;
  int last_x, last_y;
  float x2, y2;
  if (auto_scale == true) {
    for (int i = 1; i < readings; i++ ) {
      if (DataArray[i] >= maxYscale) maxYscale = DataArray[i];
      if (DataArray[i] <= minYscale) minYscale = DataArray[i];
    }
    maxYscale = round(maxYscale + auto_scale_margin); // Auto scale the graph and round to the nearest value defined, default was Y1Max
    Y1Max = round(maxYscale + 0.5);
    if (minYscale != 0) minYscale = round(minYscale - auto_scale_margin); // Auto scale the graph and round to the nearest value defined, default was Y1Min
    Y1Min = round(minYscale);
  }
  // Prevent division by zero - ensure Y1Max > Y1Min
  if (Y1Max <= Y1Min) {
    Y1Max = Y1Min + 10;  // Add minimum range
  }
  // Draw the graph
  last_x = x_pos + 1;
  last_y = y_pos + (Y1Max - constrain(DataArray[1], Y1Min, Y1Max)) / (Y1Max - Y1Min) * gheight;
  drawRect(x_pos, y_pos, gwidth + 3, gheight + 2, DarkGrey);
  drawString(x_pos - 20 + gwidth / 2 + title_x_offset, y_pos - 28, title, CENTER);

  // Draw alternating background bands FIRST (very light grey)
  for (int spacing = 0; spacing < y_minor_axis; spacing++) {
    if (spacing % 2 == 1) {  // Alternate bands
      int bandY = y_pos + (gheight * spacing / y_minor_axis);
      int bandH = gheight / y_minor_axis;
      fillRect(x_pos + 1, bandY, gwidth, bandH, 0xDD);  // Very light grey background
    }
  }
  // Draw dashed horizontal grid lines
  const int number_of_dashes = 20;
  for (int spacing = 0; spacing < y_minor_axis; spacing++) {
    for (int j = 0; j < number_of_dashes; j++) {
      drawFastHLine((x_pos + 3 + j * gwidth / number_of_dashes), y_pos + (gheight * spacing / y_minor_axis), gwidth / (2 * number_of_dashes), Grey);
    }
  }
  // Draw data lines ON TOP
  for (int gx = 0; gx < readings; gx++) {
    x2 = x_pos + gx * gwidth / (readings - 1) - 1 ; // max_readings is the global variable that sets the maximum data that can be plotted
    y2 = y_pos + (Y1Max - constrain(DataArray[gx], Y1Min, Y1Max)) / (Y1Max - Y1Min) * gheight + 1;
    if (barchart_mode) {
      // Thin bars (2px wide) like line thickness
      fillRect(x2, y2, 2, y_pos + gheight - y2 + 2, Black);
    } else {
      drawLine(last_x, last_y - 1, x2, y2 - 1, Black); // Two lines for hi-res display
      drawLine(last_x, last_y, x2, y2, Black);
    }
    last_x = x2;
    last_y = y2;
  }
  // Draw axis values
  for (int spacing = 0; spacing <= y_minor_axis; spacing++) {
    float axisValue = Y1Max - (float)(Y1Max - Y1Min) / y_minor_axis * spacing + 0.01;
    // Use 0 decimals for integer values (like 0, 20, 40, etc.)
    if (axisValue < 0.1) {
      drawString(x_pos - 10, y_pos + gheight * spacing / y_minor_axis - 5, "0", RIGHT);
    }
    else if (axisValue < 5 || title == TXT_PRESSURE_IN) {
      drawString(x_pos - 10, y_pos + gheight * spacing / y_minor_axis - 5, String(axisValue, 1), RIGHT);
    }
    else
    {
      if (Y1Min < 1 && Y1Max < 10) {
        drawString(x_pos - 3, y_pos + gheight * spacing / y_minor_axis - 5, String(axisValue, 1), RIGHT);
      }
      else {
        drawString(x_pos - 7, y_pos + gheight * spacing / y_minor_axis - 5, String(axisValue, 0), RIGHT);
      }
    }
  }
  // Day labels - calculate based on hours_span if provided, otherwise use readings
  int numDays;
  if (hours_span > 0) {
    numDays = hours_span / 24;
  } else {
    numDays = readings / 8;  // For forecast (8 readings per day = 3-hour intervals)
  }
  if (numDays < 1) numDays = 1;
  for (int i = 0; i < numDays; i++) {
    int sectionWidth = gwidth / numDays;
    int labelX = x_pos + sectionWidth * i + sectionWidth / 2;
    drawString(labelX, y_pos + gheight + 2, String(i) + "d", CENTER);
    if (i < numDays - 1) {
      drawFastVLine(x_pos + sectionWidth * (i + 1), y_pos, gheight, Grey);
    }
  }
}

// History graph - 48H shows full details, weekly shows only midnight lines
// PosArray contains normalized positions (0.0 to 1.0) based on real timestamps
void DrawHistoryGraph(int x_pos, int y_pos, int gwidth, int gheight, float Y1Min, float Y1Max, String title, String unit, float DataArray[], float PosArray[], int readings, boolean auto_scale, boolean barchart_mode, int hours_span, boolean weekly_view, int start_hour) {
#define y_minor_axis_hist 5
  setFont(OpenSans9B);
  float maxYscale = -10000;
  float minYscale =  10000;
  int last_x, last_y;
  float x2, y2;
  bool isWeeklyView = weekly_view;

  // Calculate min/max from data
  for (int i = 0; i < readings; i++) {
    if (DataArray[i] > maxYscale) maxYscale = DataArray[i];
    if (DataArray[i] < minYscale) minYscale = DataArray[i];
  }
  float dataMin = minYscale;
  float dataMax = maxYscale;

  if (auto_scale == true) {
    Y1Max = round(maxYscale + 0.5);
    if (minYscale != 0) Y1Min = round(minYscale - 0.5);
    else Y1Min = 0;
  }
  if (Y1Max <= Y1Min) {
    Y1Max = Y1Min + 10;
  }

  // Current value and trend (only for 48H view)
  float currentValue = DataArray[readings - 1];
  float trendValue = 0;
  if (readings > 6) {
    trendValue = currentValue - DataArray[readings - 7];
  }

  // Draw graph frame
  last_x = x_pos + PosArray[0] * gwidth - 1;
  last_y = y_pos + (Y1Max - constrain(DataArray[0], Y1Min, Y1Max)) / (Y1Max - Y1Min) * gheight;
  drawRect(x_pos, y_pos, gwidth + 3, gheight + 2, DarkGrey);

  // Title - different for 48H vs weekly
  if (isWeeklyView) {
    // Simple title for weekly
    drawString(x_pos + gwidth / 2, y_pos - 28, title + " (" + unit + ")", CENTER);
  } else {
    // 48H: title with current value and trend arrow
    String titleWithCurrent = title + " ";
    if (currentValue < 10 && currentValue > -10) {
      titleWithCurrent += String(currentValue, 1);
    } else {
      titleWithCurrent += String((int)currentValue);
    }
    titleWithCurrent += unit;
    drawString(x_pos + gwidth / 2, y_pos - 28, titleWithCurrent, CENTER);

    // Draw trend arrow after title - adjust position based on title length
    int arrowX = x_pos + gwidth / 2 + 85;
    int arrowY = y_pos - 18;
    // Fine-tune per graph (compare with all language variants)
    if (title == TXT_GRAPH_TEMP) { arrowX += 15; }
    else if (title == TXT_GRAPH_PRESSURE) { arrowX += 10; arrowY -= 2; }
    else if (title == TXT_GRAPH_HUMIDITY) { arrowY -= 2; }
    else if (title == TXT_GRAPH_RAIN) { arrowY -= 2; }
    if (trendValue > 0.5) {
      // Upward arrow
      fillRect(arrowX - 1, arrowY + 2, 3, 8, Black);  // Shaft
      fillTriangle(arrowX, arrowY - 2, arrowX - 4, arrowY + 3, arrowX + 4, arrowY + 3, Black);  // Head
    } else if (trendValue < -0.5) {
      // Downward arrow
      fillRect(arrowX - 1, arrowY - 2, 3, 8, Black);  // Shaft
      fillTriangle(arrowX, arrowY + 10, arrowX - 4, arrowY + 5, arrowX + 4, arrowY + 5, Black);  // Head
    } else {
      // Rightward arrow (stable/no change)
      fillRect(arrowX - 5, arrowY + 2, 10, 3, Black);  // Shaft
      fillTriangle(arrowX + 8, arrowY + 3, arrowX + 2, arrowY - 1, arrowX + 2, arrowY + 7, Black);  // Head
    }
  }

  // Draw alternating background bands
  for (int spacing = 0; spacing < y_minor_axis_hist; spacing++) {
    if (spacing % 2 == 1) {
      int bandY = y_pos + (gheight * spacing / y_minor_axis_hist);
      int bandH = gheight / y_minor_axis_hist;
      fillRect(x_pos + 1, bandY, gwidth, bandH, 0xDD);
    }
  }

  // Draw dashed horizontal grid lines
  const int number_of_dashes = 20;
  for (int spacing = 0; spacing < y_minor_axis_hist; spacing++) {
    for (int j = 0; j < number_of_dashes; j++) {
      drawFastHLine((x_pos + 3 + j * gwidth / number_of_dashes), y_pos + (gheight * spacing / y_minor_axis_hist), gwidth / (2 * number_of_dashes), Grey);
    }
  }

  // Draw vertical lines based on view type
  if (hours_span > 0) {
    float pixelsPerHour = (float)gwidth / hours_span;

    if (isWeeklyView) {
      // Weekly: only midnight lines - iterate through all hours in span
      int dayNum = 0;  // Day counter starting from 0
      setFont(OpenSans8B);
      for (int hoursFromStart = 1; hoursFromStart <= hours_span; hoursFromStart++) {
        int hourOfDay = (start_hour + hoursFromStart) % 24;
        if (hourOfDay == 0) {  // Midnight
          int lineX = x_pos + (int)(hoursFromStart * pixelsPerHour);
          if (lineX >= x_pos + 5 && lineX <= x_pos + gwidth - 5) {
            drawFastVLine(lineX - 1, y_pos, gheight, DarkGrey);
            drawFastVLine(lineX, y_pos, gheight, DarkGrey);
            drawFastVLine(lineX + 1, y_pos, gheight, DarkGrey);
            // Day number below the line (0, 1, 2, 3...)
            drawString(lineX, y_pos + gheight + 4, String(dayNum), CENTER);
          }
          dayNum++;
        }
      }
      setFont(OpenSans9B);  // Restore font
    } else {
      // 48H: lines every 6 hours
      for (int hoursFromStart = 1; hoursFromStart <= hours_span; hoursFromStart++) {
        int hourOfDay = (start_hour + hoursFromStart) % 24;
        if (hourOfDay % 6 == 0) {  // 0, 6, 12, 18
          int lineX = x_pos + (int)(hoursFromStart * pixelsPerHour);
          if (lineX >= x_pos && lineX <= x_pos + gwidth) {
            if (hourOfDay == 0) {
              // Midnight - thick line
              drawFastVLine(lineX - 1, y_pos, gheight, DarkGrey);
              drawFastVLine(lineX, y_pos, gheight, DarkGrey);
              drawFastVLine(lineX + 1, y_pos, gheight, DarkGrey);
            } else {
              // 6, 12, 18 hours
              drawFastVLine(lineX, y_pos, gheight, Grey);
            }
          }
        }
      }
    }
  }

  // Draw data lines ON TOP - use real timestamp positions from PosArray
  for (int gx = 0; gx < readings; gx++) {
    // Use normalized position from PosArray (0.0 to 1.0) for X coordinate
    x2 = x_pos + PosArray[gx] * gwidth - 1;
    y2 = y_pos + (Y1Max - constrain(DataArray[gx], Y1Min, Y1Max)) / (Y1Max - Y1Min) * gheight + 1;
    if (barchart_mode) {
      fillRect(x2, y2, 2, y_pos + gheight - y2 + 2, Black);
    } else {
      drawLine(last_x, last_y - 1, x2, y2 - 1, Black);
      drawLine(last_x, last_y, x2, y2, Black);
    }
    last_x = x2;
    last_y = y2;
  }

  // Draw Y axis values
  for (int spacing = 0; spacing <= y_minor_axis_hist; spacing++) {
    float axisValue = Y1Max - (float)(Y1Max - Y1Min) / y_minor_axis_hist * spacing + 0.01;
    if (axisValue < 0.1) {
      drawString(x_pos - 10, y_pos + gheight * spacing / y_minor_axis_hist - 5, "0", RIGHT);
    } else if (axisValue < 5) {
      drawString(x_pos - 10, y_pos + gheight * spacing / y_minor_axis_hist - 5, String(axisValue, 1), RIGHT);
    } else {
      drawString(x_pos - 7, y_pos + gheight * spacing / y_minor_axis_hist - 5, String(axisValue, 0), RIGHT);
    }
  }

  // Hour labels and min/max only for 48H view
  if (!isWeeklyView && hours_span > 0) {
    setFont(OpenSans8B);
    float pixelsPerHour = (float)gwidth / hours_span;

    // Draw only 0, 6, 12, 18 hour labels where they fall in the data range
    for (int hoursFromStart = 1; hoursFromStart <= hours_span; hoursFromStart++) {
      int hourOfDay = (start_hour + hoursFromStart) % 24;
      if (hourOfDay == 0 || hourOfDay == 6 || hourOfDay == 12 || hourOfDay == 18) {
        int labelX = x_pos + (int)(hoursFromStart * pixelsPerHour);
        // Skip if too close to edges
        if (labelX >= x_pos + 15 && labelX <= x_pos + gwidth - 15) {
          drawString(labelX, y_pos + gheight + 2, String(hourOfDay), CENTER);
        }
      }
    }

    // Min/Max labels below graph
    String minStr = "min:" + (dataMin < 10 ? String(dataMin, 1) : String((int)dataMin));
    String maxStr = "max:" + (dataMax < 10 ? String(dataMax, 1) : String((int)dataMax));
    drawString(x_pos + 3, y_pos + gheight + 14, minStr, LEFT);
    drawString(x_pos + gwidth - 3, y_pos + gheight + 14, maxStr, RIGHT);
  }
}

void drawString(int x, int y, String text, alignment align) {
  char * data  = const_cast<char*>(text.c_str());
  int  x1, y1; //the bounds of x,y and w and h of the variable 'text' in pixels.
  int w, h;
  int xx = x, yy = y;
  get_text_bounds(&currentFont, data, &xx, &yy, &x1, &y1, &w, &h, NULL);
  if (align == RIGHT)  x = x - w;
  if (align == CENTER) x = x - w / 2;
  int cursor_y = y + h;
  write_string(&currentFont, data, &x, &cursor_y, framebuffer);
}

// Get text width in pixels using current font
int getTextWidthPixels(String text) {
  char* data = const_cast<char*>(text.c_str());
  int x = 0, y = 0, x1, y1, w, h;
  get_text_bounds(&currentFont, data, &x, &y, &x1, &y1, &w, &h, NULL);
  return w;
}

// Word wrap text and draw centered lines - returns number of lines drawn
// Font must be set before calling this function
int drawWrappedText(int centerX, int startY, int maxWidth, int lineHeight, const char* text, int maxLines) {
  String remaining = text;
  int lines = 0;
  int currentY = startY;

  while (remaining.length() > 0 && lines < maxLines) {
    // Check if remaining text fits in one line
    if (getTextWidthPixels(remaining) <= maxWidth) {
      drawString(centerX, currentY, remaining, CENTER);
      return lines + 1;
    }

    // Binary search for optimal break point
    int low = 1, high = remaining.length(), breakPoint = 1;
    while (low <= high) {
      int mid = (low + high) / 2;
      if (getTextWidthPixels(remaining.substring(0, mid)) <= maxWidth) {
        breakPoint = mid;
        low = mid + 1;
      } else {
        high = mid - 1;
      }
    }

    // Find last space before breakPoint for clean word break
    int spacePos = breakPoint;
    while (spacePos > 0 && remaining.charAt(spacePos - 1) != ' ') {
      spacePos--;
    }
    if (spacePos > 0) breakPoint = spacePos;

    // Draw line and advance
    String line = remaining.substring(0, breakPoint);
    line.trim();
    drawString(centerX, currentY, line, CENTER);

    remaining = remaining.substring(breakPoint);
    remaining.trim();
    currentY += lineHeight;
    lines++;
  }

  return lines;
}

// Common screen header with date and time
void drawScreenHeader() {
  setFont(OpenSans10B);
  drawString(300, 15, String(LastUpdateDate) + "  @ " + String(LastUpdateTime), LEFT);
}

void fillCircle(int x, int y, int r, uint8_t color) {
  epd_fill_circle(x, y, r, color, framebuffer);
}

void drawFastHLine(int16_t x0, int16_t y0, int length, uint16_t color) {
  epd_draw_hline(x0, y0, length, color, framebuffer);
}

void drawFastVLine(int16_t x0, int16_t y0, int length, uint16_t color) {
  epd_draw_vline(x0, y0, length, color, framebuffer);
}

void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
  epd_write_line(x0, y0, x1, y1, color, framebuffer);
}

void drawCircle(int x0, int y0, int r, uint8_t color) {
  epd_draw_circle(x0, y0, r, color, framebuffer);
}

void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  epd_draw_rect(x, y, w, h, color, framebuffer);
}

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  epd_fill_rect(x, y, w, h, color, framebuffer);
}

// drawQRCode moved to qr_codes.h

void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                  int16_t x2, int16_t y2, uint16_t color) {
  epd_fill_triangle(x0, y0, x1, y1, x2, y2, color, framebuffer);
}

void drawPixel(int x, int y, uint8_t color) {
  epd_draw_pixel(x, y, color, framebuffer);
}

void setFont(GFXfont const &font) {
  currentFont = font;
}

void edp_update() {
  epd_draw_grayscale_image(epd_full_screen(), framebuffer); // Update the screen
}

// ==================== TOUCH SCREEN FUNCTIONS ====================

// Screen 2: Current conditions detail
void DisplayCurrentDetailScreen() {

  // Date/Time same format and position as main screen
  drawScreenHeader();

  // Title
  setFont(OpenSans18B);
  drawString(SCREEN_WIDTH / 2, 50, TXT_CURRENT_CONDITIONS, CENTER);
  drawFastHLine(150, 85, SCREEN_WIDTH - 300, DarkGrey);

  String tempUnit = (Units == "M") ? "°C" : "°F";

  // === TOP SECTION: Temperature, Pressure, Humidity ===
  int col1X = 230, col2X = 510, col3X = 760;
  int topY = 110;

  // Arrow position for trend indicators (shaft + head style, like main screen but larger)
  int arrowY = topY + 15; // Centered with text

  // Temperature - value large, unit small aligned at bottom
  setFont(OpenSans24B);
  drawString(col1X, topY, String(WxConditions[0].Temperature, 1), RIGHT);
  setFont(OpenSans14B);
  drawString(col1X + 5, topY + 10, tempUnit, LEFT);
  // Temperature trend arrow (compare current vs forecast)
  int tempArrowX = col1X + 65;
  if (WxConditions[0].Temperature < WxForecast[0].Temperature) {
    // Up arrow (will get warmer)
    fillRect(tempArrowX - 3, arrowY + 3, 6, 16, Grey);  // Shaft
    fillTriangle(tempArrowX, arrowY - 5, tempArrowX - 8, arrowY + 5, tempArrowX + 8, arrowY + 5, Grey);  // Head
  } else if (WxConditions[0].Temperature > WxForecast[0].Temperature) {
    // Down arrow (will get cooler)
    fillRect(tempArrowX - 3, arrowY - 10, 6, 16, Grey);  // Shaft
    fillTriangle(tempArrowX, arrowY + 12, tempArrowX - 8, arrowY + 2, tempArrowX + 8, arrowY + 2, Grey);  // Head
  } else {
    // Right arrow (stable)
    fillRect(tempArrowX - 10, arrowY - 3, 18, 6, Grey);  // Shaft horizontal
    fillTriangle(tempArrowX + 14, arrowY, tempArrowX + 4, arrowY - 8, tempArrowX + 4, arrowY + 8, Grey);  // Head
  }
  setFont(OpenSans12B);
  drawString(col1X - 25, topY + 50, TXT_MAX + " " + String(WxConditions[0].High, 0) + " / " + TXT_MIN + " " + String(WxConditions[0].Low, 0), CENTER);

  // Pressure - value large, unit small aligned at bottom
  setFont(OpenSans24B);
  float pressValue = (Units == "M") ? WxConditions[0].Pressure : WxConditions[0].Pressure * HPA_TO_INHG;
  String pressUnit = (Units == "M") ? (currentLang == 0 ? "mb" : "hPa") : "inHg";
  drawString(col2X, topY, String(pressValue, 0), RIGHT);
  setFont(OpenSans14B);
  drawString(col2X + 5, topY + 10, pressUnit, LEFT);
  // Pressure trend arrow (using existing trend calculation)
  int pressArrowX = col2X + 75;
  if (WxConditions[0].Trend == "+") {
    // Up arrow (pressure rising)
    fillRect(pressArrowX - 3, arrowY + 3, 6, 16, Grey);  // Shaft
    fillTriangle(pressArrowX, arrowY - 5, pressArrowX - 8, arrowY + 5, pressArrowX + 8, arrowY + 5, Grey);  // Head
  } else if (WxConditions[0].Trend == "-") {
    // Down arrow (pressure falling)
    fillRect(pressArrowX - 3, arrowY - 10, 6, 16, Grey);  // Shaft
    fillTriangle(pressArrowX, arrowY + 12, pressArrowX - 8, arrowY + 2, pressArrowX + 8, arrowY + 2, Grey);  // Head
  } else {
    // Right arrow (stable)
    fillRect(pressArrowX - 10, arrowY - 3, 18, 6, Grey);  // Shaft horizontal
    fillTriangle(pressArrowX + 14, arrowY, pressArrowX + 4, arrowY - 8, pressArrowX + 4, arrowY + 8, Grey);  // Head
  }
  setFont(OpenSans12B);
  drawString(col2X - 20, topY + 50, TXT_PRESSURE, CENTER);

  // Humidity - value large, unit small aligned at bottom
  setFont(OpenSans24B);
  drawString(col3X, topY, String(WxConditions[0].Humidity, 0), RIGHT);
  setFont(OpenSans14B);
  drawString(col3X + 5, topY + 10, "%", LEFT);
  // Humidity trend arrow (compare current vs forecast)
  int humArrowX = col3X + 50;
  if (WxConditions[0].Humidity < WxForecast[0].Humidity) {
    // Up arrow (will get more humid)
    fillRect(humArrowX - 3, arrowY + 3, 6, 16, Grey);  // Shaft
    fillTriangle(humArrowX, arrowY - 5, humArrowX - 8, arrowY + 5, humArrowX + 8, arrowY + 5, Grey);  // Head
  } else if (WxConditions[0].Humidity > WxForecast[0].Humidity) {
    // Down arrow (will get less humid)
    fillRect(humArrowX - 3, arrowY - 10, 6, 16, Grey);  // Shaft
    fillTriangle(humArrowX, arrowY + 12, humArrowX - 8, arrowY + 2, humArrowX + 8, arrowY + 2, Grey);  // Head
  } else {
    // Right arrow (stable)
    fillRect(humArrowX - 10, arrowY - 3, 18, 6, Grey);  // Shaft horizontal
    fillTriangle(humArrowX + 14, arrowY, humArrowX + 4, arrowY - 8, humArrowX + 4, arrowY + 8, Grey);  // Head
  }
  setFont(OpenSans12B);
  drawString(col3X - 15, topY + 50, TXT_HUMIDITY, CENTER);

  // === MIDDLE SECTION: Weather Icon + Details ===
  drawFastHLine(50, 190, SCREEN_WIDTH - 100, DarkGrey);

  // Left: Weather icon (extra large)
  iconScaleOverride = XLarge;
  DisplayConditionsSection(220, 300, WxConditions[0].Icon, LargeIcon);
  iconScaleOverride = 0;

  // Weather description below icon (max 25 chars per line, 3 lines)
  setFont(OpenSans10B);
  String wxDesc = WxConditions[0].Forecast0;

  // Test with longest possible description (uncomment to test):
  //wxDesc = "Tormentas electricas moderadas con lluvia intensa y granizo";

  const int maxCharsDetail = 25;
  String descLine1 = "", descLine2 = "", descLine3 = "";
  String remaining = wxDesc;

  // Line 1
  if (remaining.length() <= maxCharsDetail) {
    descLine1 = remaining;
    remaining = "";
  } else {
    int breakPos = maxCharsDetail;
    for (int i = maxCharsDetail - 1; i >= 0; i--) {
      if (remaining.charAt(i) == ' ') { breakPos = i; break; }
    }
    descLine1 = remaining.substring(0, breakPos);
    remaining = remaining.substring(breakPos + 1);
  }

  // Line 2
  if (remaining.length() > 0) {
    if (remaining.length() <= maxCharsDetail) {
      descLine2 = remaining;
      remaining = "";
    } else {
      int breakPos = maxCharsDetail;
      for (int i = maxCharsDetail - 1; i >= 0; i--) {
        if (remaining.charAt(i) == ' ') { breakPos = i; break; }
      }
      descLine2 = remaining.substring(0, breakPos);
      remaining = remaining.substring(breakPos + 1);
    }
  }

  // Line 3
  if (remaining.length() > 0) {
    if (remaining.length() <= maxCharsDetail) {
      descLine3 = remaining;
    } else {
      int breakPos = maxCharsDetail;
      for (int i = maxCharsDetail - 1; i >= 0; i--) {
        if (remaining.charAt(i) == ' ') { breakPos = i; break; }
      }
      descLine3 = remaining.substring(0, breakPos);
    }
  }

  drawString(220, 385, TitleCase(descLine1), CENTER);
  if (descLine2.length() > 0) drawString(220, 405, TitleCase(descLine2), CENTER);
  if (descLine3.length() > 0) drawString(220, 425, TitleCase(descLine3), CENTER);

  // Right: Wind, Gusts, Visibility, Cloudiness, Feels Like, Dew Point (6 rows)
  int rightColLabel = 645, rightColValue = 685;
  int row1Y = 210, row2Y = 248, row3Y = 286, row4Y = 324, row5Y = 362, row6Y = 400;

  setFont(OpenSans12B);
  drawString(rightColLabel, row1Y, TXT_WIND + ":", RIGHT);
  drawString(rightColLabel, row2Y - 8, TXT_GUSTS + ":", RIGHT);
  drawString(rightColLabel, row3Y, TXT_VISIBILITY + ":", RIGHT);
  drawString(rightColLabel, row4Y, TXT_CLOUDINESS + ":", RIGHT);
  drawString(rightColLabel, row5Y, TXT_FEELS_LIKE + ":", RIGHT);
  drawString(rightColLabel, row6Y, TXT_DEWPOINT + ":", RIGHT);

  setFont(OpenSans14B);
  float visKm = WxConditions[0].Visibility / 1000.0;
  String windDir = WindDegToOrdinalDirection(WxConditions[0].Winddir);
  drawString(rightColValue, row1Y - 3, windDir + " - " + String(WxConditions[0].Windspeed, 1) + ((Units == "M") ? " km/h" : " mph"), LEFT);
  drawString(rightColValue, row2Y - 7, String(WxConditions[0].Gust, 1) + ((Units == "M") ? " km/h" : " mph"), LEFT);
  drawString(rightColValue, row3Y - 3, String(visKm, 1) + " km", LEFT);
  drawString(rightColValue, row4Y - 3, String(WxConditions[0].Cloudcover) + "%", LEFT);
  drawString(rightColValue, row5Y - 3, String(WxConditions[0].Feelslike, 1) + tempUnit, LEFT);
  drawString(rightColValue, row6Y - 3, String(WxConditions[0].Dewpoint, 1) + tempUnit, LEFT);

  // === BOTTOM SECTION: Sunrise, Sunset, Rain Prob, UV, AQI (5 columns) ===
  int lineY = 450;
  drawFastHLine(50, lineY, SCREEN_WIDTH - 100, DarkGrey);

  // X positions for 5 columns evenly distributed
  int sunriseX = 100;
  int sunsetX = 290;
  int rainX = 480;
  int uvX = 670;
  int aqiX = 860;

  // Y positions
  int valY = 470;
  int labelOffsetY = 33;

  // Values
  setFont(OpenSans12B);
  drawString(sunriseX, valY, convertTo24Hour(WxConditions[0].SunriseStr), CENTER);
  drawString(sunsetX, valY, convertTo24Hour(WxConditions[0].SunsetStr), CENTER);
  // Show daily rainfall total from WeatherAPI (already converted to inches if Imperial)
  String rainUnit = (Units == "M") ? "mm" : "in";
  drawString(rainX, valY, String(WxConditions[0].DailyRainfall, 1) + rainUnit, CENTER);

  // UV with level (uppercase, smaller font, raised 5px)
  setFont(OpenSans12B);
  float uv = WxConditions[0].UVIndex;
  String uvLevel;
  if (uv < 3) uvLevel = " " + TXT_UV_LOW_S;
  else if (uv < 6) uvLevel = " " + TXT_UV_MOD_S;
  else if (uv < 8) uvLevel = " " + TXT_UV_HIGH_S;
  else if (uv < 11) uvLevel = " " + TXT_UV_VHIGH_S;
  else uvLevel = " " + TXT_UV_EXT_S;
  uvLevel.toUpperCase();
  drawString(uvX, valY - 5, String(uv, 1) + uvLevel, CENTER);

  // AQI (uppercase, smaller font)
  String aqiText;
  switch(WxConditions[0].AQI) {
    case 1: aqiText = TXT_AQI_GOOD; break;
    case 2: aqiText = TXT_AQI_FAIR; break;
    case 3: aqiText = TXT_AQI_MODERATE; break;
    case 4: aqiText = TXT_AQI_POOR; break;
    case 5: aqiText = TXT_AQI_VERY_POOR; break;
    default: aqiText = "--";
  }
  aqiText.toUpperCase();
  drawString(aqiX, valY - 2, aqiText, CENTER);

  // Labels
  setFont(OpenSans12B);
  drawString(sunriseX, valY + labelOffsetY, TXT_SUNRISE, CENTER);
  drawString(sunsetX, valY + labelOffsetY, TXT_SUNSET, CENTER);
  drawString(rainX, valY + labelOffsetY, TXT_RAIN_TODAY, CENTER);
  drawString(uvX, valY + labelOffsetY, TXT_UV_INDEX, CENTER);
  drawString(aqiX, valY + labelOffsetY, TXT_AQI, CENTER);

  // Status indicators (SD, Battery, WiFi) - same position as main screen
  DisplayStatusSection(600, 33, wifi_signal);
}

// Helper function to get quality level for each pollutant (5 levels)
String getPollutantQuality(const char* pollutant, float value) {
  if (strcmp(pollutant, "PM2.5") == 0) {
    if (value <= 10) return TXT_AQI_GOOD;
    if (value <= 25) return TXT_AQI_FAIR;
    if (value <= 50) return TXT_AQI_MODERATE;
    if (value <= 75) return TXT_AQI_POOR;
    return TXT_AQI_VERY_POOR;
  }
  if (strcmp(pollutant, "PM10") == 0) {
    if (value <= 20) return TXT_AQI_GOOD;
    if (value <= 50) return TXT_AQI_FAIR;
    if (value <= 100) return TXT_AQI_MODERATE;
    if (value <= 200) return TXT_AQI_POOR;
    return TXT_AQI_VERY_POOR;
  }
  if (strcmp(pollutant, "O3") == 0) {
    if (value <= 60) return TXT_AQI_GOOD;
    if (value <= 100) return TXT_AQI_FAIR;
    if (value <= 140) return TXT_AQI_MODERATE;
    if (value <= 180) return TXT_AQI_POOR;
    return TXT_AQI_VERY_POOR;
  }
  if (strcmp(pollutant, "CO") == 0) {
    if (value <= 4400) return TXT_AQI_GOOD;
    if (value <= 9400) return TXT_AQI_FAIR;
    if (value <= 12400) return TXT_AQI_MODERATE;
    if (value <= 15400) return TXT_AQI_POOR;
    return TXT_AQI_VERY_POOR;
  }
  if (strcmp(pollutant, "NO2") == 0) {
    if (value <= 40) return TXT_AQI_GOOD;
    if (value <= 90) return TXT_AQI_FAIR;
    if (value <= 120) return TXT_AQI_MODERATE;
    if (value <= 230) return TXT_AQI_POOR;
    return TXT_AQI_VERY_POOR;
  }
  if (strcmp(pollutant, "SO2") == 0) {
    if (value <= 40) return TXT_AQI_GOOD;
    if (value <= 80) return TXT_AQI_FAIR;
    if (value <= 380) return TXT_AQI_MODERATE;
    if (value <= 800) return TXT_AQI_POOR;
    return TXT_AQI_VERY_POOR;
  }
  return "--";
}

// Helper function to get max scale for each pollutant
// Fixed scale for all except CO (which uses dynamic scale)
float getPollutantMaxScale(const char* pollutant, float value) {
  // Fixed scales based on max possible values
  if (strcmp(pollutant, "PM2.5") == 0) return 100;
  if (strcmp(pollutant, "PM10") == 0) return 300;
  if (strcmp(pollutant, "O3") == 0) return 250;
  if (strcmp(pollutant, "NO2") == 0) return 300;
  if (strcmp(pollutant, "SO2") == 0) return 1000;

  // CO uses dynamic scale
  if (strcmp(pollutant, "CO") == 0) {
    float standardMax = 20000;
    float maxVal;
    if (value < standardMax * 0.2) {
      maxVal = value * 5;
      if (maxVal < 100) maxVal = 100;
    } else if (value > standardMax * 0.9) {
      maxVal = value * 1.2;
    } else {
      maxVal = standardMax;
    }
    return maxVal;
  }

  return 100;
}

// Draw a single air quality bar graph (compact for 6 columns)
void drawAirQualityBar(int x, int y, const char* name, float value, const char* unit) {
  // Dimensions - 40px wider bars
  int barWidth = 70;
  int barHeight = 220;

  // Get scale and quality
  float maxScale = getPollutantMaxScale(name, value);
  String quality = getPollutantQuality(name, value);

  // Calculate bar fill height
  float fillRatio = value / maxScale;
  if (fillRatio > 1.0) fillRatio = 1.0;
  int fillHeight = (int)(barHeight * fillRatio);

  // Draw title (name) - larger font
  setFont(OpenSans12B);
  drawString(x + 40, y, name, CENTER);

  // Frame position (offset right to leave space for scale on left)
  int frameX = x;
  int frameY = y + 22;

  // Draw scale on left side (10 divisions for more lines) with tick marks
  setFont(OpenSans8B);
  int scaleX = frameX - 5;
  int numDivisions = 10;
  for (int i = 0; i <= numDivisions; i++) {
    int scaleY = frameY + 5 + (barHeight * i / numDivisions);
    float scaleVal = maxScale * (numDivisions - i) / numDivisions;

    // Major ticks (with numbers) - longer, with dotted line across graph
    if (i % 2 == 0) {
      drawFastHLine(frameX - 6, scaleY + 5, 8, Black);
      // Dotted line across the bar area (gray)
      if (i > 0 && i < numDivisions) {
        for (int dx = 0; dx < barWidth; dx += 4) {
          drawPixel(frameX + 5 + dx, scaleY + 5, Grey);
          drawPixel(frameX + 6 + dx, scaleY + 5, Grey);
        }
      }
      String scaleStr;
      if (maxScale >= 1000) {
        scaleStr = String((int)scaleVal);
      } else if (maxScale >= 100) {
        scaleStr = String((int)scaleVal);
      } else {
        scaleStr = String(scaleVal, 0);
      }
      drawString(scaleX, scaleY + 4, scaleStr, RIGHT);
    } else {
      // Minor ticks - shorter, same color and thickness
      drawFastHLine(frameX - 3, scaleY + 5, 5, Black);
    }
  }

  // Draw frame (rectangle)
  drawRect(frameX, frameY, barWidth + 10, barHeight + 10, Black);

  // Draw bar fill (darker gray) from bottom
  int barX = frameX + 5;
  int barY = frameY + 5;
  int fillStartY = barY + barHeight - fillHeight;
  fillRect(barX, fillStartY, barWidth, fillHeight, DarkGrey);

  // Draw value below bar (larger font)
  setFont(OpenSans12B);
  int valueY = frameY + barHeight + 25;
  String valueStr;
  if (value >= 1000) {
    valueStr = String((int)value);
  } else if (value >= 100) {
    valueStr = String(value, 0);
  } else {
    valueStr = String(value, 1);
  }
  drawString(x + 40, valueY, valueStr, CENTER);

  // Draw unit below value
  setFont(OpenSans8B);
  drawString(x + 40, valueY + 18, unit, CENTER);

  // Draw quality label below unit (larger font, uppercase, +10px separation)
  setFont(OpenSans10B);
  quality.toUpperCase();
  drawString(x + 40, valueY + 48, quality, CENTER);
}

// Screen: Air Quality Details
void DisplayAirQualityScreen() {
  drawScreenHeader();

  int centerX = SCREEN_WIDTH / 2;

  // Title - moved up 20px
  int titleY = 55;
  int titleLineY = 90;

  setFont(OpenSans18B);
  drawString(centerX, titleY, TXT_AIR_QUALITY, CENTER);
  drawFastHLine(150, titleLineY, SCREEN_WIDTH - 300, Grey);

  // Layout: 6 columns in single row (wider bars) - moved 20px right
  int colWidth = 150;
  int startX = 75;  // Left margin for scale numbers (+20px)
  int rowY = 120;

  // All 6 pollutants in one row: PM2.5, PM10, O3, CO, NO2, SO2
  drawAirQualityBar(startX + colWidth * 0, rowY, "PM2.5", WxConditions[0].PM2_5, "ug/m3");
  drawAirQualityBar(startX + colWidth * 1, rowY, "PM10", WxConditions[0].PM10, "ug/m3");
  drawAirQualityBar(startX + colWidth * 2, rowY, "O3", WxConditions[0].O3, "ug/m3");
  drawAirQualityBar(startX + colWidth * 3, rowY, "CO", WxConditions[0].CO, "ug/m3");
  drawAirQualityBar(startX + colWidth * 4, rowY, "NO2", WxConditions[0].NO2, "ug/m3");
  drawAirQualityBar(startX + colWidth * 5, rowY, "SO2", WxConditions[0].SO2, "ug/m3");

  // Line below quality labels (moved up 10px)
  int bottomLineY = 470;
  drawFastHLine(100, bottomLineY, SCREEN_WIDTH - 200, Grey);

  // ICA and UV below the line - larger font
  setFont(OpenSans16B);

  // ICA (Air Quality Index) - uppercase with colon
  String aqiDesc;
  switch(WxConditions[0].AQI) {
    case 1: aqiDesc = TXT_AQI_GOOD; break;
    case 2: aqiDesc = TXT_AQI_FAIR; break;
    case 3: aqiDesc = TXT_AQI_MODERATE; break;
    case 4: aqiDesc = TXT_AQI_POOR; break;
    case 5: aqiDesc = TXT_AQI_VERY_POOR; break;
    default: aqiDesc = "--";
  }
  aqiDesc.toUpperCase();
  String icaText = TXT_AQI + " :  " + String(WxConditions[0].AQI) + " - " + aqiDesc;
  drawString(centerX - 175, bottomLineY + 15, icaText, CENTER);

  // UV Index - uppercase with colon (+30px separation)
  String uvLevel;
  float uv = WxConditions[0].UVIndex;
  if (uv < 3) uvLevel = TXT_UV_LOW;
  else if (uv < 6) uvLevel = TXT_UV_MODERATE;
  else if (uv < 8) uvLevel = TXT_UV_HIGH;
  else if (uv < 11) uvLevel = TXT_UV_VERY_HIGH;
  else uvLevel = TXT_UV_EXTREME;
  uvLevel.toUpperCase();
  String uvText = "UV :  " + String(uv, 1) + " - " + uvLevel;
  drawString(centerX + 175, bottomLineY + 15, uvText, CENTER);
}

// Calendar screens moved to calendar.h
// (DisplayCalendarScreen and DisplayCalendarYearScreen are in calendar.h)

// Screen 3: Extended forecast
void DisplayForecastScreen() {
  // Date/Time same format and position as main screen
  drawScreenHeader();

  // Title
  setFont(OpenSans18B);
  drawString(SCREEN_WIDTH / 2, 50, TXT_5DAY_FORECAST, CENTER);
  drawFastHLine(150, 85, SCREEN_WIDTH - 300, Grey);

  // === SECTION 1: Hourly forecast - 8 columns ===
  setFont(OpenSans12B);
  drawString(SCREEN_WIDTH / 2, 100, TXT_NEXT_24H, CENTER);

  int hourlyY = 145;
  int hourlyWidth = SCREEN_WIDTH / 8;  // 120 pixels each
  int startIdx = getFirstRelevantForecastIndex();
  for (int i = 0; i < min(8, max_readings - startIdx); i++) {
    int dataIdx = startIdx + i;
    int xPos = hourlyWidth / 2 + i * hourlyWidth;

    setFont(OpenSans10B);
    String timeStr = ConvertUnixTime(WxForecast[dataIdx].Dt + WxConditions[0].Timezone).substring(0, 5);
    drawString(xPos, hourlyY, timeStr, CENTER);

    DisplayConditionsSection(xPos, hourlyY + 65, WxForecast[dataIdx].Icon, SmallIcon);

    setFont(OpenSans10B);
    drawString(xPos, hourlyY + 105, String(WxForecast[dataIdx].High, 0) + "°/" + String(WxForecast[dataIdx].Low, 0) + "°", CENTER);
  }

  // Divider
  drawFastHLine(50, 275, SCREEN_WIDTH - 100, Grey);

  // === SECTION 2: 3-day forecast - 3 columns (WeatherAPI provides 3 days) ===
  setFont(OpenSans12B);
  drawString(SCREEN_WIDTH / 2, 300, TXT_5DAY_OUTLOOK, CENTER);

  int dailyY = 335;
  int dailyWidth = SCREEN_WIDTH / 3;  // 320 pixels each

  for (int day = 0; day < 3; day++) {
    int baseIdx = day * 8;
    if (baseIdx >= max_readings) break;

    int xPos = dailyWidth / 2 + day * dailyWidth;

    // Day name
    time_t tm = WxForecast[baseIdx].Dt;
    struct tm *dt = localtime(&tm);
    setFont(OpenSans12B);
    drawString(xPos, dailyY, weekday_D[dt->tm_wday], CENTER);

    // Calculate high/low/rain for the day
    float dayHigh = -100, dayLow = 100, dayRain = 0;
    String dayIcon = "";
    for (int h = 0; h < 8 && (baseIdx + h) < max_readings; h++) {
      int idx = baseIdx + h;
      if (WxForecast[idx].High > dayHigh) dayHigh = WxForecast[idx].High;
      if (WxForecast[idx].Low < dayLow) dayLow = WxForecast[idx].Low;
      dayRain += WxForecast[idx].Rainfall;
      if (h == 4 || h == 5) dayIcon = WxForecast[idx].Icon;
    }
    if (dayIcon == "") dayIcon = WxForecast[baseIdx].Icon;

    // Weather icon
    DisplayConditionsSection(xPos, dailyY + 70, dayIcon, SmallIcon);

    // Temperature
    setFont(OpenSans12B);
    drawString(xPos, dailyY + 120, String(dayHigh, 0) + "°/" + String(dayLow, 0) + "°", CENTER);

    // Rain if any
    if (dayRain > 0.1) {
      setFont(OpenSans10B);
      drawString(xPos, dailyY + 150, String(dayRain, 1) + "mm", CENTER);
    }
  }
}

// Screen 4: Expanded graphs
void DisplayGraphsScreen() {
  // Date/Time same format and position as main screen
  drawScreenHeader();

  // Title
  setFont(OpenSans18B);
  drawString(SCREEN_WIDTH / 2, 50, TXT_WEATHER_TRENDS, CENTER);
  drawFastHLine(150, 85, SCREEN_WIDTH - 300, Grey);

  // Prepare data (WeatherAPI provides 3 days = 24 readings)
  for (int r = 0; r < forecast_readings; r++) {
    if (Units == "I") {
      pressure_readings[r] = WxForecast[r].Pressure * HPA_TO_INHG;
      rain_readings[r] = WxForecast[r].Rainfall * MM_TO_INCHES;
      snow_readings[r] = WxForecast[r].Snowfall * MM_TO_INCHES;
    } else {
      pressure_readings[r] = WxForecast[r].Pressure;
      rain_readings[r] = WxForecast[r].Rainfall;
      snow_readings[r] = WxForecast[r].Snowfall;
    }
    temperature_readings[r] = WxForecast[r].Temperature;
    humidity_readings[r] = WxForecast[r].Humidity;
  }

  // 2x2 grid of graphs (same layout as History screen)
  int gwidth = 390;
  int gheight = 140;
  int gx1 = 65, gx2 = 530;
  int gy1 = 120, gy2 = 320;

  // Temperature - top left
  if (Units == "M")
    DrawGraph(gx1, gy1, gwidth, gheight, 10, 40, TXT_TEMPERATURE_C, temperature_readings, forecast_readings, autoscale_on, barchart_off);
  else
    DrawGraph(gx1, gy1, gwidth, gheight, 50, 95, TXT_TEMPERATURE_F, temperature_readings, forecast_readings, autoscale_on, barchart_off);

  // Pressure - top right
  if (Units == "M")
    DrawGraph(gx2, gy1, gwidth, gheight, 900, 1050, TXT_PRESSURE_HPA, pressure_readings, forecast_readings, autoscale_on, barchart_off);
  else
    DrawGraph(gx2, gy1, gwidth, gheight, 26, 31, TXT_PRESSURE_IN, pressure_readings, forecast_readings, autoscale_on, barchart_off);

  // Humidity - bottom left
  DrawGraph(gx1, gy2, gwidth, gheight, 0, 100, TXT_HUMIDITY_PERCENT, humidity_readings, forecast_readings, autoscale_off, barchart_off);

  // Rain/Snow - bottom right (thin bars)
  if (SumOfPrecip(rain_readings, forecast_readings) >= SumOfPrecip(snow_readings, forecast_readings)) {
    if (Units == "M")
      DrawGraph(gx2, gy2, gwidth, gheight, 0, 30, TXT_RAINFALL_MM, rain_readings, forecast_readings, autoscale_on, barchart_on);
    else
      DrawGraph(gx2, gy2, gwidth, gheight, 0, 1.2, TXT_RAINFALL_IN, rain_readings, forecast_readings, autoscale_on, barchart_on);
  } else {
    if (Units == "M")
      DrawGraph(gx2, gy2, gwidth, gheight, 0, 30, TXT_SNOWFALL_MM, snow_readings, forecast_readings, autoscale_on, barchart_on);
    else
      DrawGraph(gx2, gy2, gwidth, gheight, 0, 1.2, TXT_SNOWFALL_IN, snow_readings, forecast_readings, autoscale_on, barchart_on);
  }

  // Footer
  setFont(OpenSans12B);
  drawString(SCREEN_WIDTH / 2, 510, TXT_NEXT_DAYS, CENTER);
}

// Screen 5: System Information
void DisplayInfoScreen() {

  // Title
  setFont(OpenSans18B);
  drawString(SCREEN_WIDTH / 2, 35, TXT_SYSTEM_INFO, CENTER);
  drawFastHLine(100, 67, SCREEN_WIDTH - 200, Black);
  drawFastHLine(100, 70, SCREEN_WIDTH - 200, Grey);

  // Page indicator
  setFont(OpenSans10B);
  drawString(SCREEN_WIDTH / 2, 515, "0 / 5", CENTER);

  drawInfoNavArrow(50);

  int col1X = 50;    // Labels left
  int col2X = 230;   // Values left
  int col3X = 500;   // Labels right
  int col4X = 680;   // Values right

  setFont(OpenSans10B);

  // === LEFT COLUMN - Explicit Y coordinates ===

  // Section 1: System Info
  drawString(col1X, 100, TXT_VERSION, LEFT);
  drawString(col2X, 100, version, LEFT);

  drawString(col1X, 128, TXT_COMPILED, LEFT);
  drawString(col2X, 128, String(__DATE__) + " " + String(__TIME__), LEFT);

  drawString(col1X, 156, TXT_FREE_HEAP, LEFT);
  drawString(col2X, 156, String(ESP.getFreeHeap() / 1024) + " KB", LEFT);

  drawString(col1X, 184, TXT_TOTAL_HEAP, LEFT);
  drawString(col2X, 184, String(ESP.getHeapSize() / 1024) + " KB", LEFT);

  drawString(col1X, 212, "PSRAM:", LEFT);
  drawString(col2X, 212, String(ESP.getFreePsram() / 1024) + " KB", LEFT);

  // Section 2: WiFi Info (with gap)
  drawString(col1X, 255, "WiFi SSID:", LEFT);
  drawString(col2X, 255, WiFi.isConnected() ? WiFi.SSID() : TXT_DISCONNECTED, LEFT);

  drawString(col1X, 283, "IP:", LEFT);
  drawString(col2X, 283, WiFi.isConnected() ? WiFi.localIP().toString() : "--", LEFT);

  drawString(col1X, 311, TXT_WIFI_SIGNAL, LEFT);
  drawString(col2X, 311, WiFi.isConnected() ? String(WiFi.RSSI()) + " dBm" : "--", LEFT);

  // Section 3: Location (with gap)
  drawString(col1X, 354, TXT_CITY, LEFT);
  drawString(col2X, 354, String(config.city), LEFT);

  drawString(col1X, 382, TXT_COORDINATES, LEFT);
  drawString(col2X, 382, String(config.latitude) + ", " + String(config.longitude), LEFT);

  drawString(col1X, 410, TXT_TIMEZONE, LEFT);
  drawString(col2X, 410, String(config.timezone), LEFT);

  // === RIGHT COLUMN - Explicit Y coordinates ===

  // Section 1: Config
  drawString(col3X, 100, TXT_LANGUAGE, LEFT);
  String langStr = strcmp(config.language, "ES") == 0 ? "Espanol" :
                   strcmp(config.language, "EN") == 0 ? "English" : "Francais";
  drawString(col4X, 100, langStr, LEFT);

  drawString(col3X, 128, TXT_UNITS, LEFT);
  drawString(col4X, 128, strcmp(config.units, "M") == 0 ? TXT_METRIC : TXT_IMPERIAL, LEFT);

  drawString(col3X, 156, TXT_HEMISPHERE, LEFT);
  drawString(col4X, 156, strcmp(config.hemisphere, "north") == 0 ? TXT_NORTH : TXT_SOUTH, LEFT);

  drawString(col3X, 184, TXT_FORECAST, LEFT);
  drawString(col4X, 184, String(config.forecast_days) + " " + TXT_DAYS, LEFT);

  // Section 2: System Config (with gap)
  drawString(col3X, 227, TXT_UPDATE_INTERVAL, LEFT);
  drawString(col4X, 227, String(config.update_interval) + " min", LEFT);

  drawString(col3X, 255, "Timeout:", LEFT);
  drawString(col4X, 255, String(config.sleep_timeout) + " " + TXT_SEC, LEFT);

  drawString(col3X, 283, TXT_ON_SLEEP, LEFT);
  drawString(col4X, 283, config.keep_screen_on_sleep ? TXT_KEEP_SCREEN : TXT_GO_MAIN, LEFT);

  // Section 3: API Info (with gap)
  drawString(col3X, 326, "API Key:", LEFT);
  String apiKeyMasked = String(config.api_key).substring(0, 8) + "...";
  drawString(col4X, 326, apiKeyMasked, LEFT);

  // Web Config URL
  drawString(col3X, 369, TXT_WEB_CONFIG, LEFT);
  if (WiFi.isConnected()) {
    drawString(col4X, 369, "http://" + WiFi.localIP().toString(), LEFT);
  } else {
    drawString(col4X, 369, TXT_WIFI_DISCONNECTED, LEFT);
  }

  // SD Card info (with gap)
  drawString(col3X, 425, "SD Card:", LEFT);
  if (sdCardAvailable) {
    uint64_t totalMB = SD.cardSize() / (1024 * 1024);
    uint64_t usedMB = SD.usedBytes() / (1024 * 1024);
    uint64_t freeMB = totalMB - usedMB;
    drawString(col4X, 425, String(usedMB) + "/" + String(totalMB) + " MB", LEFT);
  } else {
    drawString(col4X, 425, "--", LEFT);
  }

  // Bluetooth config button at bottom left
  setFont(OpenSans12B);
  int bleBtnX = 50;
  int btnY = 451;
  for (int i = 0; i < 5; i++) {
    drawRect(bleBtnX + i, btnY + i, 180 - i*2, 45 - i*2, Black);
  }
  drawString(bleBtnX + 90, btnY + 11, "Bluetooth", CENTER);

  // Clean display button at bottom center
  int btnX = SCREEN_WIDTH/2 - 120;
  // Draw thick border (5px) like toggle buttons
  for (int i = 0; i < 5; i++) {
    drawRect(btnX + i, btnY + i, 240 - i*2, 45 - i*2, Black);
  }
  drawString(SCREEN_WIDTH/2, btnY + 8, TXT_CLEAN_SCREEN, CENTER);

  // Radio button at bottom right
  int radioBtnX = 730;
  for (int i = 0; i < 5; i++) {
    drawRect(radioBtnX + i, btnY + i, 180 - i*2, 45 - i*2, Black);
  }
  drawString(radioBtnX + 90, btnY + 11, "Radio", CENTER);

  // Right arrow (next page - Features) - like calendar arrows
  int arrowY = 50;
  int arrowSize = 25;
  int rightArrowX = SCREEN_WIDTH - 60;
  fillTriangle(rightArrowX, arrowY,
               rightArrowX - arrowSize, arrowY - arrowSize/2,
               rightArrowX - arrowSize, arrowY + arrowSize/2, Black);
  fillTriangle(rightArrowX - 20, arrowY,
               rightArrowX - arrowSize - 20, arrowY - arrowSize/2,
               rightArrowX - arrowSize - 20, arrowY + arrowSize/2, Black);

}

// Screen: Features/Specifications
// Helper function to draw navigation arrow
void drawInfoNavArrow(int arrowY) {
  int arrowSize = 25;
  int rightArrowX = SCREEN_WIDTH - 60;
  fillTriangle(rightArrowX, arrowY,
               rightArrowX - arrowSize, arrowY - arrowSize/2,
               rightArrowX - arrowSize, arrowY + arrowSize/2, Black);
  fillTriangle(rightArrowX - 20, arrowY,
               rightArrowX - arrowSize - 20, arrowY - arrowSize/2,
               rightArrowX - arrowSize - 20, arrowY + arrowSize/2, Black);
}

// Screen: Features 1/2 - Hardware
void DisplayInfoFeatures1Screen() {

  // Title
  setFont(OpenSans18B);
  drawString(SCREEN_WIDTH / 2, 35, TXT_INFO_HARDWARE, CENTER);
  drawFastHLine(100, 87, SCREEN_WIDTH - 200, Black);
  drawFastHLine(100, 90, SCREEN_WIDTH - 200, Grey);

  // Page indicator
  setFont(OpenSans10B);
  drawString(SCREEN_WIDTH / 2, 515, "1 / 5", CENTER);

  drawInfoNavArrow(50);

  int y = 120;
  int lineH = 26;
  int col1 = 50;
  int col2 = 530;

  setFont(OpenSans10B);

  // Left column
  drawString(col1, y, TXT_HW_MICRO, LEFT);
  y += lineH;
  drawString(col1, y, TXT_HW_CPU, LEFT);
  y += lineH;
  drawString(col1, y, TXT_HW_FLASH, LEFT);
  y += lineH;
  drawString(col1, y, TXT_HW_PSRAM, LEFT);
  y += lineH * 2;
  drawString(col1, y, TXT_HW_WIFI, LEFT);
  y += lineH;
  drawString(col1, y, TXT_HW_BLE, LEFT);
  y += lineH * 2;
  drawString(col1, y, TXT_HW_ACTIVE, LEFT);
  y += lineH;
  drawString(col1, y, TXT_HW_SLEEP, LEFT);

  // Right column
  y = 120;
  drawString(col2, y, TXT_HW_DISPLAY, LEFT);
  y += lineH;
  drawString(col2, y, TXT_HW_RES, LEFT);
  y += lineH;
  drawString(col2, y, TXT_HW_COLORS, LEFT);
  y += lineH;
  drawString(col2, y, TXT_HW_REFRESH, LEFT);
  y += lineH;
  drawString(col2, y, TXT_HW_ANGLE, LEFT);
  y += lineH;
  drawString(col2, y, TXT_HW_TOUCH, LEFT);
  y += lineH * 2;
  drawString(col2, y, TXT_HW_BATT, LEFT);
  y += lineH;
  drawString(col2, y, TXT_HW_SD, LEFT);

  // Footer
  setFont(OpenSans8B);
  drawFastHLine(100, 500, SCREEN_WIDTH - 200, Grey);
}

// Screen: Features 2/2 - Software & API
void DisplayInfoFeatures2Screen() {

  // Title
  setFont(OpenSans18B);
  drawString(SCREEN_WIDTH / 2, 35, TXT_INFO_SOFTWARE, CENTER);
  drawFastHLine(100, 87, SCREEN_WIDTH - 200, Black);
  drawFastHLine(100, 90, SCREEN_WIDTH - 200, Grey);

  // Page indicator
  setFont(OpenSans10B);
  drawString(SCREEN_WIDTH / 2, 515, "2 / 5", CENTER);

  drawInfoNavArrow(50);

  int y = 120;
  int lineH = 26;
  int col1 = 50;
  int col2 = 530;

  setFont(OpenSans10B);

  // Left column - Software
  drawString(col1, y, TXT_SW_SCREENS, LEFT);
  y += lineH;
  drawString(col1, y, TXT_SW_MULTIWIFI, LEFT);
  y += lineH;
  drawString(col1, y, TXT_SW_HISTSD, LEFT);
  y += lineH;
  drawString(col1, y, TXT_SW_HISTINT, LEFT);
  y += lineH;
  drawString(col1, y, TXT_SW_UPDATE, LEFT);
  y += lineH;
  drawString(col1, y, TXT_SW_LANGS, LEFT);
  y += lineH * 2;
  drawString(col1, y, TXT_SW_APMODE, LEFT);
  y += lineH;
  drawString(col1, y, TXT_SW_PORTAL, LEFT);
  y += lineH;
  drawString(col1, y, TXT_SW_PASS, LEFT);

  // Right column - API
  y = 120;
  drawString(col2, y, TXT_SW_API, LEFT);
  y += lineH * 2;
  drawString(col2, y, TXT_SW_ENDPOINT, LEFT);
  y += lineH;
  drawString(col2, y, TXT_SW_FORECAST, LEFT);
  y += lineH;
  drawString(col2, y, TXT_SW_UVAQI, LEFT);
  y += lineH;
  drawString(col2, y, TXT_SW_ONECALL, LEFT);
  y += lineH * 2;
  drawString(col2, y, TXT_SW_LIMIT, LEFT);
  y += lineH * 2;
  drawString(col2, y, TXT_SW_STORAGE, LEFT);

  // Footer
  setFont(OpenSans8B);
  drawFastHLine(100, 500, SCREEN_WIDTH - 200, Grey);
}

// Screen: Update Information
void DisplayInfoUpdateScreen() {

  // Title
  setFont(OpenSans18B);
  drawString(SCREEN_WIDTH / 2, 35, TXT_INFO_UPDATE, CENTER);
  drawFastHLine(100, 87, SCREEN_WIDTH - 200, Black);
  drawFastHLine(100, 90, SCREEN_WIDTH - 200, Grey);

  // Page indicator
  setFont(OpenSans10B);
  drawString(SCREEN_WIDTH / 2, 515, "3 / 5", CENTER);

  drawInfoNavArrow(50);

  int y = 120;
  int lineH = 28;
  int col1 = 50;
  int col2 = 330;  // Moved right for longer labels

  // Station Information Section
  setFont(OpenSans12B);
  drawString(col1, y, TXT_UPD_STATION, LEFT);
  drawFastHLine(col1, y + 18, 280, Grey);
  y += 32;

  setFont(OpenSans10B);

  // Last update time
  drawString(col1, y, TXT_UPD_LAST_UPDATE, LEFT);
  drawString(col2, y, String(LastUpdateDate) + "  @ " + String(LastUpdateTime), LEFT);
  y += lineH;

  // Station ID
  drawString(col1, y, TXT_UPD_STATION_ID, LEFT);
  if (strlen(config.location_id) > 0) {
    drawString(col2, y, String(config.location_id), LEFT);
  } else {
    drawString(col2, y, "--", LEFT);
  }
  y += lineH;

  // Station name
  drawString(col1, y, TXT_UPD_STATION_NAME, LEFT);
  drawString(col2, y, String(StationName), LEFT);
  y += lineH;

  // Coordinates
  drawString(col1, y, TXT_UPD_COORDINATES, LEFT);
  char coordStr[30];
  sprintf(coordStr, "%.4f, %.4f", StationLat, StationLon);
  drawString(col2, y, String(coordStr), LEFT);
  y += lineH;

  // Station data time (from epoch)
  drawString(col1, y, TXT_UPD_DATA_TIME, LEFT);
  if (StationDataEpoch > 0) {
    time_t stationTime = StationDataEpoch;
    struct tm* tm_info = localtime(&stationTime);
    char timeStr[30];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M", tm_info);
    drawString(col2, y, String(timeStr), LEFT);
  } else {
    drawString(col2, y, "--", LEFT);
  }
  y += lineH * 2;

  // Network Information Section
  setFont(OpenSans12B);
  drawString(col1, y, TXT_UPD_NETWORK, LEFT);
  drawFastHLine(col1, y + 18, 250, Grey);
  y += 32;

  setFont(OpenSans10B);

  // WiFi name
  drawString(col1, y, TXT_UPD_WIFI_NAME, LEFT);
  drawString(col2, y, WiFi.isConnected() ? WiFi.SSID() : "--", LEFT);
  y += lineH;

  // Signal level
  drawString(col1, y, TXT_UPD_SIGNAL, LEFT);
  if (WiFi.isConnected()) {
    drawString(col2, y, String(WiFi.RSSI()) + " dBm", LEFT);
  } else {
    drawString(col2, y, "--", LEFT);
  }
  y += lineH;

  // IP Address
  drawString(col1, y, TXT_UPD_IP, LEFT);
  drawString(col2, y, WiFi.isConnected() ? WiFi.localIP().toString() : "--", LEFT);

  // Footer
  setFont(OpenSans8B);
  drawFastHLine(100, 500, SCREEN_WIDTH - 200, Grey);
}

// Screen: Quick Help
void DisplayInfoHelpScreen() {

  // Title
  setFont(OpenSans18B);
  drawString(SCREEN_WIDTH / 2, 35, TXT_INFO_HELP, CENTER);
  drawFastHLine(100, 87, SCREEN_WIDTH - 200, Black);
  drawFastHLine(100, 90, SCREEN_WIDTH - 200, Grey);

  // Page indicator
  setFont(OpenSans10B);
  drawString(SCREEN_WIDTH / 2, 515, "4 / 5", CENTER);

  drawInfoNavArrow(50);

  int y = 120;
  int lineH = 26;
  int col1 = 50;
  int col2 = 530;

  setFont(OpenSans12B);

  // Navigation tips
  drawString(col1, y, TXT_HELP_NAV, LEFT);
  drawFastHLine(col1, y + 18, 220, Grey);
  y += 28;

  setFont(OpenSans10B);
  drawString(col1, y, TXT_HELP_ZONE, LEFT);
  y += lineH;
  drawString(col1, y, TXT_HELP_ICONS, LEFT);
  y += lineH;
  drawString(col1, y, TXT_HELP_GRAPH_LEFT, LEFT);
  y += lineH;
  drawString(col1, y, TXT_HELP_GRAPH_RIGHT, LEFT);
  y += lineH;
  drawString(col1, y, TXT_HELP_BATTERY, LEFT);
  y += lineH;
  drawString(col1, y, TXT_HELP_CALENDAR, LEFT);
  y += lineH + 10;

  // Config tips
  setFont(OpenSans12B);
  drawString(col1, y, TXT_HELP_INITIAL, LEFT);
  drawFastHLine(col1, y + 18, 230, Grey);
  y += 28;

  setFont(OpenSans10B);
  drawString(col1, y, TXT_HELP_STEP1, LEFT);
  y += lineH;
  drawString(col1, y, TXT_HELP_STEP2, LEFT);
  y += lineH;
  drawString(col1, y, TXT_HELP_STEP3, LEFT);
  y += lineH;
  drawString(col1, y, TXT_HELP_STEP4, LEFT);
  y += lineH;
  drawString(col1, y, TXT_HELP_STEP5, LEFT);
  y += lineH + 10;

  // Right column - Troubleshooting
  y = 120;

  setFont(OpenSans12B);
  drawString(col2, y, TXT_HELP_TROUBLE, LEFT);
  drawFastHLine(col2, y + 18, 250, Grey);
  y += 28;

  setFont(OpenSans10B);
  drawString(col2, y, TXT_HELP_NOWIFI, LEFT);
  y += lineH;
  drawString(col2, y, TXT_HELP_NODATA, LEFT);
  y += lineH;
  drawString(col2, y, TXT_HELP_GHOST, LEFT);
  y += lineH;
  drawString(col2, y, TXT_HELP_TOUCH, LEFT);
  y += lineH * 3;

  setFont(OpenSans12B);
  drawString(col2, y, TXT_HELP_BOOT, LEFT);
  drawFastHLine(col2, y + 18, 200, Grey);
  y += 28;

  setFont(OpenSans10B);
  drawString(col2, y, TXT_HELP_BOOT1, LEFT);
  y += lineH;
  drawString(col2, y, TXT_HELP_BOOT2, LEFT);
  y += lineH;
  drawString(col2, y, TXT_HELP_BOOT3, LEFT);
  y += lineH;
  drawString(col2, y, TXT_HELP_BOOT4, LEFT);

  // Footer
  setFont(OpenSans8B);
  drawFastHLine(100, 500, SCREEN_WIDTH - 200, Grey);
}

// Screen: Credits with QR codes
void DisplayInfoCreditsScreen() {

  // Title
  setFont(OpenSans18B);
  drawString(SCREEN_WIDTH / 2, 35, TXT_INFO_CREDITS, CENTER);
  drawFastHLine(100, 67, SCREEN_WIDTH - 200, Black);
  drawFastHLine(100, 70, SCREEN_WIDTH - 200, Grey);

  // Page indicator
  setFont(OpenSans10B);
  drawString(SCREEN_WIDTH / 2, 515, "5 / 5", CENTER);

  drawInfoNavArrow(50);

  int col1 = 50;
  int col2 = 530;
  int y = 90;
  int lineH = 22;

  // Left column - Original author (required by license)
  setFont(OpenSans12B);
  drawString(col1, y, TXT_CR_AUTHOR, LEFT);
  drawFastHLine(col1, y + 18, 180, Grey);
  y += 28;

  setFont(OpenSans10B);
  drawString(col1, y, "David Bird (G6EJD)", LEFT);
  y += lineH;
  drawString(col1, y, "ESP32 e-Paper Weather Display", LEFT);
  y += lineH;
  drawString(col1, y, "Copyright 2014-2021", LEFT);
  y += lineH;
  drawString(col1, y, "github.com/G6EJD", LEFT);
  y += lineH + 10;

  // Adaptations chain
  setFont(OpenSans12B);
  drawString(col1, y, TXT_CR_ADAPTATIONS, LEFT);
  drawFastHLine(col1, y + 18, 170, Grey);
  y += 28;

  setFont(OpenSans10B);
  drawString(col1, y, TXT_CR_ADAPT1, LEFT);
  y += lineH;
  drawString(col1, y, TXT_CR_ADAPT2, LEFT);
  y += lineH;
  drawString(col1, y, TXT_CR_ADAPT3, LEFT);
  y += lineH + 10;

  setFont(OpenSans12B);
  drawString(col1, y, TXT_CR_MODS, LEFT);
  drawFastHLine(col1, y + 18, 310, Grey);
  y += 28;

  setFont(OpenSans10B);
  drawString(col1, y, TXT_CR_MOD1, LEFT);
  y += lineH;
  drawString(col1, y, TXT_CR_MOD2, LEFT);
  y += lineH;
  drawString(col1, y, TXT_CR_MOD3, LEFT);
  y += lineH;
  drawString(col1, y, TXT_CR_MOD4, LEFT);
  y += lineH;
  drawString(col1, y, TXT_CR_MOD5, LEFT);
  y += lineH;
  drawString(col1, y, TXT_CR_MOD6, LEFT);

  // Right column
  y = 90;

  setFont(OpenSans12B);
  drawString(col2, y, TXT_CR_HW, LEFT);
  drawFastHLine(col2, y + 18, 120, Grey);
  y += 28;

  setFont(OpenSans10B);
  drawString(col2, y, "LilyGo T5 4.7\" S3 EPD touch", LEFT);
  y += lineH + 10;

  setFont(OpenSans12B);
  drawString(col2, y, TXT_CR_LIBS, LEFT);
  drawFastHLine(col2, y + 18, 120, Grey);
  y += 28;

  setFont(OpenSans10B);
  drawString(col2, y, "* EPD47 - LilyGO/DFRobot", LEFT);
  y += lineH;
  drawString(col2, y, "* ArduinoJson - B. Blanchon", LEFT);
  y += lineH;
  drawString(col2, y, "* ESP32 Arduino - Espressif", LEFT);
  y += lineH + 10;

  setFont(OpenSans12B);
  drawString(col2, y, TXT_CR_LINKS, LEFT);
  drawFastHLine(col2, y + 18, 100, Grey);
  y += 28;

  setFont(OpenSans10B);
  drawString(col2, y, "github.com/xe1e", LEFT);
  y += lineH + 10;

  // QR Code for github.com/xe1e
  drawQRCode(QR_XE1E, col2 + 5, y, 4);  // 29x4 = 116px + margins

  // License notice (required by original license)
  setFont(OpenSans8B);
  drawString(col1, 490, "Solo uso personal, no comercial. Original: David Bird (G6EJD) - Ver Licence.txt", LEFT);
}

// Screen 6: Weather History (real recorded data)
void DisplayHistoryScreen() {
  // Date/Time same format and position as main screen
  drawScreenHeader();

  // Title
  setFont(OpenSans18B);
  drawString(SCREEN_WIDTH / 2, 50, TXT_HISTORY, CENTER);
  drawFastHLine(150, 85, SCREEN_WIDTH - 300, Grey);

  // Toggle button top right - text with thick border
  setFont(OpenSans12B);
  String toggleText = historyShowWeek ? TXT_48H : TXT_1WEEK;
  int btnX = SCREEN_WIDTH - 105;
  int btnY = 35;
  // Draw thick border (5px)
  for (int i = 0; i < 5; i++) {
    drawRect(btnX + i, btnY + i, 90 - i*2, 32 - i*2, Black);
  }
  drawString(btnX + 45, btnY + 6, toggleText, CENTER);

  if (historyCount < 2) {
    setFont(OpenSans12B);
    drawString(SCREEN_WIDTH / 2, 300, TXT_NOT_ENOUGH_DATA, CENTER);
    drawString(SCREEN_WIDTH / 2, 340, TXT_HISTORY_FILLS, CENTER);
    return;
  }

  // Prepare data arrays
  #define HISTORY_GRAPH_READINGS 100
  float hist_temp[HISTORY_GRAPH_READINGS];
  float hist_hum[HISTORY_GRAPH_READINGS];
  float hist_press[HISTORY_GRAPH_READINGS];
  float hist_rain[HISTORY_GRAPH_READINGS];
  float hist_pos[HISTORY_GRAPH_READINGS];  // Normalized positions based on real timestamps

  // Get readings based on selected view (48h or 168h = 1 week)
  int requestedHours = historyShowWeek ? 168 : 48;
  int readings = getReadingsForHours(requestedHours, hist_temp, hist_hum, hist_press, hist_rain, hist_pos, HISTORY_GRAPH_READINGS);

  // Get actual hours span from timestamps (not assuming 10-min intervals)
  int actualHours = (int)getHistoryHoursSpan(requestedHours);
  if (actualHours < 1) actualHours = 1;

  // Get start hour from oldest data timestamp
  int startHour = getHistoryStartHour(requestedHours);

  if (readings < 2) {
    setFont(OpenSans12B);
    drawString(SCREEN_WIDTH / 2, 300, TXT_INSUFFICIENT_DATA, CENTER);
    return;
  }

  // Draw 4 graphs (2x2 layout)
  int gwidth = 390;
  int gheight = 140;
  int gx1 = 65, gx2 = 530;
  int gy1 = 120, gy2 = 320;

  // Temperature - top left
  DrawHistoryGraph(gx1, gy1, gwidth, gheight, 0, 40, TXT_GRAPH_TEMP, Units == "M" ? "°C" : "°F", hist_temp, hist_pos, readings, autoscale_on, barchart_off, actualHours, historyShowWeek, startHour);

  // Pressure - top right
  DrawHistoryGraph(gx2, gy1, gwidth, gheight, 900, 1050, TXT_GRAPH_PRESSURE, Units == "M" ? (currentLang == 0 ? "mb" : "hPa") : "in", hist_press, hist_pos, readings, autoscale_on, barchart_off, actualHours, historyShowWeek, startHour);

  // Humidity - bottom left
  DrawHistoryGraph(gx1, gy2, gwidth, gheight, 0, 100, TXT_GRAPH_HUMIDITY, "%", hist_hum, hist_pos, readings, autoscale_off, barchart_off, actualHours, historyShowWeek, startHour);

  // Rainfall - bottom right
  DrawHistoryGraph(gx2, gy2, gwidth, gheight, 0, 10, TXT_GRAPH_RAIN, Units == "M" ? "mm" : "in", hist_rain, hist_pos, readings, autoscale_on, barchart_on, actualHours, historyShowWeek, startHour);

  // Footer - readings count and storage type
  setFont(OpenSans10B);
  String histInfo = String(historyCount) + " " + TXT_READINGS + " / " + String(getHistoryDays(), 1) + " " + TXT_DAYS;

  // Show storage indicator if SD available
  if (sdCardAvailable) {
    int totalSD = getTotalHistoryReadings();
    histInfo = histInfo + " [SD: " + String(totalSD) + "]";
  }

  drawString(SCREEN_WIDTH / 2, 505, histInfo, CENTER);
}

