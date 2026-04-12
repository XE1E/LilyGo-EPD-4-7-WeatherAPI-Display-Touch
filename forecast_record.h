#ifndef FORECAST_RECORD_H_
#define FORECAST_RECORD_H_

#include <Arduino.h>

typedef struct { // For current Day and Day 1, 2, 3, etc
  int      Dt;
  String   Period;
  String   Icon;
  String   Trend;
  String   Main0;
  String   Forecast0;
  String   Forecast1;
  String   Forecast2;
  String   Description;
  String   Time;
  String   Country;
  float    lat;
  float    lon;
  float    Temperature;
  float    Feelslike;
  float    Humidity;
  float    High;
  float    Low;
  float    Winddir;
  String   WinddirStr;    // Wind direction as text ("N", "NE", etc.)
  float    Windspeed;
  float    Gust;          // Wind gust speed
  float    Rainfall;
  float    DailyRainfall;  // Total precipitation for the day (from API)
  float    Snowfall;
  float    Pop;           // Probability of precipitation (0-1)
  int      ChanceOfRain;  // Chance of rain %
  int      ChanceOfSnow;  // Chance of snow %
  float    Pressure;
  int      Cloudcover;
  int      Visibility;
  float    Dewpoint;      // Dew point temperature
  int      Sunrise;       // Unix timestamp (kept for compatibility)
  int      Sunset;        // Unix timestamp (kept for compatibility)
  String   SunriseStr;    // "06:40 AM" format from WeatherAPI
  String   SunsetStr;     // "06:48 PM" format from WeatherAPI
  String   Moonrise;      // Moon rise time
  String   Moonset;       // Moon set time
  String   MoonPhase;     // "Waxing Crescent", "Full Moon", etc.
  int      MoonIllum;     // Moon illumination % (0-100)
  int      Timezone;
  // UV Index and Air Quality
  float    UVIndex;
  int      AQI;           // Air Quality Index (1-5 EPA)
  int      AQI_DEFRA;     // UK DEFRA index
  float    PM2_5;         // PM2.5 ug/m3
  float    PM10;          // PM10 ug/m3
  float    CO;            // Carbon monoxide ug/m3
  float    NO2;           // Nitrogen dioxide ug/m3
  float    O3;            // Ozone ug/m3
  float    SO2;           // Sulphur dioxide ug/m3
  // Location
  String   Region;        // State/Region
  // Condition code for short description
  int      ConditionCode; // WeatherAPI condition code
} Forecast_record_type;

#endif /* ifndef FORECAST_RECORD_H_ */
