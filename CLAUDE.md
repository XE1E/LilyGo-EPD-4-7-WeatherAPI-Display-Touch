# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ESP32-based weather display for the LilyGo EPD 4.7" e-paper display with **touch navigation**. Fetches weather data from **WeatherAPI.com** (single HTTPS call) and displays current conditions plus 3-day forecast. Features multi-WiFi support, AP mode captive portal for configuration, touch navigation across 11+ screens, and deep sleep for battery operation.

## Build & Upload

### Arduino IDE Settings
- **Board**: ESP32S3 Dev Module
- **USB CDC On Boot**: Enable
- **USB DFU On Boot**: Disable
- **Flash Size**: 16MB (128Mb)
- **Flash Mode**: QIO 80MHz
- **Partition Scheme**: 16M Flash (3M APP/9.9MB FATFS)
- **PSRAM**: OPI PSRAM
- **Upload Mode**: UART0/Hardware CDC
- **USB Mode**: Hardware CDC and JTAG

### Required Libraries
- **Board Manager**: esp32 by Espressif Systems 2.0.17
- **EPD47-master**: https://github.com/DFRobotdl/EPD47/archive/refs/heads/master.zip
- **ArduinoJson**: by Benoit Blanchon 6.19.0

Only install these two libraries in the Arduino libraries folder to avoid conflicts.

### Force Upload Mode
If upload fails, enter bootloader mode:
1. Press and hold BOOT(IO0) button
2. While holding BOOT, press RST
3. Release RST
4. Release BOOT

## Architecture

### Main Flow
`setup()` → `InitialiseSystem()` → `loadConfig()` → WiFi connect → fetch weather (HTTPS) → `DisplayWeather()` → touch navigation → deep sleep

If WiFi fails or `FORCE_AP_MODE=true`, enters AP mode with captive portal for configuration.

### Key Files
- **LilyGo-EPD-4-7-WeatherAPI-Display-Touch.ino**: Main sketch - WiFi connection, API calls, display rendering, touch navigation, sleep management
- **owm_credentials.h**: Default configuration (WiFi SSIDs, API key, location, timezone). Values can be overridden via web config
- **wifi_manager.h**: AP mode captive portal, web server, preferences storage (ESP32 NVS)
- **forecast_record.h**: `Forecast_record_type` struct for weather data (extended for WeatherAPI fields)
- **touch_handler.h**: Touch navigation logic
- **weather_narrative.h**: AI-generated weather descriptions (Groq/Llama)
- **weather_history.h**: Data storage system (SD + FFat)
- **calendar.h**: Calendar views
- **lang.h**: UI text strings (Spanish/English/French)
- **opensans*.h**: Font definitions (sizes 6-24)
- **moon.h, sunrise.h, sunset.h**: Icon bitmaps

### Configuration Storage
Web-configured values are stored in ESP32 preferences (NVS) namespace "weather" and override defaults in `owm_credentials.h`.

### AP Mode Types
- **AP_INITIAL_SETUP**: First boot, no valid config - no timeout, auto-reboot on save
- **AP_RECOVERY**: WiFi connection failed - 5 min timeout, retries WiFi after
- **AP_FORCED**: FORCE_AP_MODE=true - no timeout, for maintenance

### First Boot Detection
The device detects first boot by checking for placeholder values ("YOUR_WEATHERAPI_KEY") or empty config in NVS.

### WiFi Connection Logic
1. Check if first boot (no valid config) → AP_INITIAL_SETUP mode
2. Check FORCE_AP_MODE flag → AP_FORCED mode
3. Scans available networks
4. Matches against stored config + hardcoded credentials
5. Connects to network with strongest signal
6. Falls back to AP mode (AP_RECOVERY) if no connection
7. After 5 failed retries, enters permanent deep sleep (requires manual reset)

### Web Configuration Features
- **Test WiFi**: Scans networks to verify SSID exists, shows signal strength
- **Test API**: Validates WeatherAPI key with real API call (normal mode only)
- **Immediate Settings**: Language, units, intervals apply without reboot
- **Reboot Settings**: WiFi, API keys, location require reboot

### Display
Display is 960x540 pixels. Shows main weather screen with touch navigation to 11+ screens. Updates based on configured interval then returns to deep sleep.

## WeatherAPI Integration

### API Endpoint
```
https://api.weatherapi.com/v1/forecast.json
  ?key={API_KEY}
  &q={LAT},{LON}
  &days=3
  &aqi=yes
  &lang={LANG}
```

### Single API Call
WeatherAPI returns all data in one response (~50KB JSON):
- **location**: City, region, country, coordinates
- **current**: Temperature, humidity, pressure, wind, UV, condition, air quality
- **forecast.forecastday[]**: 3 days of hourly data
- **astro**: Sunrise, sunset, moonrise, moonset, moon phase, illumination

### Key Functions
- `obtainWeatherData()`: HTTPS call to WeatherAPI (port 443, WiFiClientSecure)
- `DecodeWeatherAPI()`: Parses JSON into WxConditions[] and WxForecast[]
- `mapWeatherAPIIcon()`: Maps WeatherAPI condition codes to OWM-style icon codes (01d, 02d, etc.)
- `TranslateMoonPhase()`: Translates English moon phase names to current language
- `DrawMoonFromAPI()`: Draws moon using illumination percentage from API

## Configuration

### AP Mode Access
- SSID: `WeatherStation-Setup`
- Password: `weather123`
- URL: `http://192.168.4.1`

### Required API Key
Get a free API key from https://www.weatherapi.com/ and set in `owm_credentials.h` or via web interface.

## Troubleshooting

### Common Issues
| Error | Cause | Fix |
|-------|-------|-----|
| Connection failed 401/403 | Invalid API key | Verify key at weatherapi.com |
| deserializeJson NoMemory | Response too large | Buffer is 64KB, alerts disabled |
| WiFi connection failed | No network | Check credentials, 2.4GHz only |

### Memory Management
- JSON buffer: 64KB (DynamicJsonDocument)
- Response read as String before parsing (more reliable with HTTPS)
- Alerts disabled (`alerts=no`) to reduce response size
