#pragma once

#include <Arduino.h>

enum WeatherIconType {
    ICON_SUNNY = 0,
    ICON_PARTLY_CLOUDY,
    ICON_CLOUDY,
    ICON_RAIN,
    ICON_SNOW,
    ICON_THUNDERSTORM,
    ICON_UNKNOWN
};

struct CurrentWeather {
    float temperature;
    int humidity;
    int weatherCode;
    char conditionText[30];
    WeatherIconType iconType;
};

struct DailyForecast {
    char date[11];        // "YYYY-MM-DD"
    char dayName[15];     // "Domingo", "Lunes", etc.
    char shortDayName[8]; // "DOM 18"
    float tempMax;
    float tempMin;
    int weatherCode;
    char conditionText[30];
    WeatherIconType iconType;
};

struct WeatherData {
    bool valid;
    unsigned long lastFetchTime;
    CurrentWeather current;
    DailyForecast forecast[2]; // forecast[0]: Hoy, forecast[1]: Mañana
};

const char* getWeatherDescription(int code);
WeatherIconType getWeatherIconType(int code);

class WeatherService {
public:
    WeatherService();
    bool fetchWeather(const char* lat, const char* lon, WeatherData& outData);
};
