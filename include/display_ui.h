#pragma once

#include <Arduino.h>
#include <IPAddress.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "time_service.h"
#include "weather_service.h"

// Colores RGB565 para interfaz gráfica
#define COLOR_BLACK         0x0000
#define COLOR_WHITE         0xFFFF
#define COLOR_RED           0xF800
#define COLOR_GREEN         0x07E0
#define COLOR_CYAN          0x07FF
#define COLOR_YELLOW        0xFFE0
#define COLOR_ORANGE        0xFD20
#define COLOR_DARKGREY      0x39E7
#define COLOR_LIGHTGREY     0xC618
#define COLOR_BG_HEADER     0x0926
#define COLOR_BORDER_CARD   0x2145
#define COLOR_TEXT_MUTED    0x8410

enum TFTDriverType {
    TFT_ST7735S_R = 0, // ST7735S o ST7735R (pantalla 1.77" o 1.8" estándar, usa initR)
    TFT_ST7735B   = 1  // ST7735B (pantalla 1.8" con controlador ST7735B, usa initB)
};

class DisplayUI {
public:
    DisplayUI(uint8_t cs, uint8_t dc, uint8_t rst);

    void begin(uint8_t rotation = 0, TFTDriverType driverType = TFT_ST7735S_R, uint8_t initTab = INITR_BLACKTAB, bool invertColors = false);
    void showBootScreen(const char* ssid);
    void updateBootProgress(const char* stepMsg, uint8_t dots);

    // Dibuja la estructura base fija (cabecera, marcos de tarjetas, separadores)
    void drawBaseLayout(const char* locationName);

    // Métodos de actualización dinámica (flicker-free)
    void updateTime(const DateTimeInfo& timeInfo);
    void updateDate(const DateTimeInfo& timeInfo);
    void updateWeather(const WeatherData& weather);
    void updateWifiStatus(bool connected, int rssi, const IPAddress& ip);
    void updateStatusBar(const char* message);

private:
    Adafruit_ST7735 _tft;
    char _lastTimeStr[6]; // "HH:MM"
    char _lastDateStr[24];
    int _lastRssi;
    bool _lastWifiState;

    void drawWeatherIcon(int x, int y, WeatherIconType type, bool small);
    void drawWifiIndicator(int x, int y, bool connected, int rssi);
};
