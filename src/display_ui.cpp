#include "display_ui.h"
#include <math.h>

DisplayUI::DisplayUI(uint8_t cs, uint8_t dc, uint8_t rst)
    : _tft(cs, dc, rst), _lastRssi(-999), _lastWifiState(false) {
    _lastTimeStr[0] = '\0';
    _lastDateStr[0] = '\0';
}

void DisplayUI::begin(uint8_t rotation, TFTDriverType driverType, uint8_t initTab, bool invertColors) {
    if (driverType == TFT_ST7735B) {
        _tft.initB();
    } else {
        _tft.initR(initTab);
    }
    _tft.setRotation(rotation);
    if (invertColors) {
        _tft.invertDisplay(true);
    }
    _tft.fillScreen(COLOR_BLACK);
}

void DisplayUI::showBootScreen(const char* ssid) {
    _tft.fillScreen(COLOR_BLACK);

    // Cabecera inicial
    _tft.fillRect(0, 0, 128, 20, COLOR_BG_HEADER);
    _tft.drawFastHLine(0, 20, 128, COLOR_CYAN);

    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_WHITE, COLOR_BG_HEADER);
    _tft.setCursor(6, 6);
    _tft.print("Weather Clock Mini");

    // Recuadro central
    _tft.drawRoundRect(4, 26, 120, 128, 4, COLOR_DARKGREY);

    _tft.setTextColor(COLOR_YELLOW, COLOR_BLACK);
    _tft.setCursor(14, 38);
    _tft.print("Iniciando...");

    _tft.setTextColor(COLOR_LIGHTGREY, COLOR_BLACK);
    _tft.setCursor(10, 60);
    _tft.print("Conectando Wi-Fi:");

    _tft.setTextColor(COLOR_CYAN, COLOR_BLACK);
    _tft.setCursor(10, 75);
    char ssidBuf[19];
    snprintf(ssidBuf, sizeof(ssidBuf), "%-18.18s", ssid);
    _tft.print(ssidBuf);

    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BLACK);
    _tft.setCursor(10, 100);
    _tft.print("NTP / Open-Meteo");
}

void DisplayUI::updateBootProgress(const char* stepMsg, uint8_t dots) {
    _tft.fillRect(6, 122, 116, 18, COLOR_BLACK);
    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_ORANGE, COLOR_BLACK);
    _tft.setCursor(8, 124);

    char msgBuf[16];
    snprintf(msgBuf, sizeof(msgBuf), "%-14.14s", stepMsg);
    _tft.print(msgBuf);

    for (uint8_t i = 0; i < (dots % 4); i++) {
        _tft.print(".");
    }
}

void DisplayUI::drawBaseLayout(const char* locationName) {
    _tft.fillScreen(COLOR_BLACK);

    // 1. Barra de Cabecera (Y: 0..15)
    _tft.fillRect(0, 0, 128, 15, COLOR_BG_HEADER);
    _tft.drawFastHLine(0, 15, 128, COLOR_CYAN);

    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_CYAN, COLOR_BG_HEADER);
    _tft.setCursor(4, 4);
    char locBuf[13];
    snprintf(locBuf, sizeof(locBuf), "%-12.12s", locationName);
    _tft.print(locBuf);

    // 2. Marco Tarjeta Reloj Digital (Y: 18..63)
    _tft.drawRoundRect(2, 18, 124, 45, 3, COLOR_DARKGREY);

    // 3. Marco Tarjeta Clima Actual (Y: 65..112)
    _tft.drawRoundRect(2, 65, 124, 47, 3, 0x2A6D);
    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_CYAN, COLOR_BLACK);
    _tft.setCursor(6, 68);
    _tft.print("CLIMA");

    // 4. Marcos Pronóstico 2 Días (Y: 114..148)
    _tft.drawRoundRect(2, 114, 60, 34, 3, COLOR_BORDER_CARD);
    _tft.drawRoundRect(66, 114, 60, 34, 3, COLOR_BORDER_CARD);

    // 5. Línea divisoria barra de estado
    _tft.drawFastHLine(0, 149, 128, COLOR_BORDER_CARD);
}

void DisplayUI::updateTime(const DateTimeInfo& timeInfo) {
    if (strncmp(_lastTimeStr, timeInfo.shortTimeStr, sizeof(_lastTimeStr)) == 0) {
        return;
    }

    strncpy(_lastTimeStr, timeInfo.shortTimeStr, sizeof(_lastTimeStr));
    _lastTimeStr[sizeof(_lastTimeStr) - 1] = '\0';

    // "HH:MM" -> 5 caracteres en tamaño 3 = 5 * 18 = 90 px
    // Centrado: (128 - 90) / 2 = 19
    _tft.setTextSize(3);
    _tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
    _tft.setCursor(19, 23);
    _tft.print(_lastTimeStr);
}

void DisplayUI::updateDate(const DateTimeInfo& timeInfo) {
    if (strncmp(_lastDateStr, timeInfo.shortDateStr, sizeof(_lastDateStr)) == 0) {
        return;
    }

    strncpy(_lastDateStr, timeInfo.shortDateStr, sizeof(_lastDateStr));
    _lastDateStr[sizeof(_lastDateStr) - 1] = '\0';

    // Limpiar línea de fecha
    _tft.fillRect(4, 50, 120, 10, COLOR_BLACK);

    // Centrar fecha: "SAB 17 OCT" (10 chars * 6 = 60 px -> X = 34)
    int len = strlen(_lastDateStr);
    int posX = (128 - (len * 6)) / 2;
    if (posX < 4) posX = 4;

    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_YELLOW, COLOR_BLACK);
    _tft.setCursor(posX, 50);
    _tft.print(_lastDateStr);
}

void DisplayUI::updateWeather(const WeatherData& weather) {
    if (!weather.valid) return;

    // 1. Humedad en cabecera de la tarjeta
    _tft.fillRect(68, 67, 56, 9, COLOR_BLACK);
    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BLACK);
    _tft.setCursor(68, 68);
    _tft.printf("%2d%% Hum", weather.current.humidity);

    // 2. Limpiar área interior del clima actual
    _tft.fillRect(4, 76, 120, 34, COLOR_BLACK);

    // 3. Icono del clima actual (radio 7 px, centrado en X=16, Y=92)
    drawWeatherIcon(16, 92, weather.current.iconType, false);

    // 4. Temperatura actual en tamaño 2
    _tft.setTextSize(2);
    _tft.setTextColor(COLOR_ORANGE, COLOR_BLACK);
    _tft.setCursor(34, 77);
    _tft.printf("%.1f C", weather.current.temperature);

    // 5. Condición en tamaño 1 (máximo 15 caracteres)
    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_LIGHTGREY, COLOR_BLACK);
    _tft.setCursor(34, 98);
    char condBuf[16];
    snprintf(condBuf, sizeof(condBuf), "%-15.15s", weather.current.conditionText);
    _tft.print(condBuf);

    // 6. Pronóstico Día Actual (Hoy)
    _tft.fillRect(4, 116, 56, 30, COLOR_BLACK);
    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_YELLOW, COLOR_BLACK);
    _tft.setCursor(6, 117);
    _tft.print(weather.forecast[0].shortDayName); // "HOY DD"

    drawWeatherIcon(13, 136, weather.forecast[0].iconType, true);

    _tft.setCursor(24, 132);
    _tft.setTextColor(COLOR_CYAN, COLOR_BLACK);
    _tft.printf("%2.0f", weather.forecast[0].tempMin);
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BLACK);
    _tft.print("/");
    _tft.setTextColor(COLOR_ORANGE, COLOR_BLACK);
    _tft.printf("%2.0f", weather.forecast[0].tempMax);

    // 7. Pronóstico Día Siguiente (Mañana)
    _tft.fillRect(68, 116, 56, 30, COLOR_BLACK);
    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_YELLOW, COLOR_BLACK);
    _tft.setCursor(70, 117);
    _tft.print(weather.forecast[1].shortDayName); // "DIA DD"

    drawWeatherIcon(77, 136, weather.forecast[1].iconType, true);

    _tft.setCursor(88, 132);
    _tft.setTextColor(COLOR_CYAN, COLOR_BLACK);
    _tft.printf("%2.0f", weather.forecast[1].tempMin);
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BLACK);
    _tft.print("/");
    _tft.setTextColor(COLOR_ORANGE, COLOR_BLACK);
    _tft.printf("%2.0f", weather.forecast[1].tempMax);
}

void DisplayUI::drawWifiIndicator(int x, int y, bool connected, int rssi) {
    _tft.fillRect(x, y, 14, 11, COLOR_BG_HEADER);

    if (!connected) {
        _tft.drawLine(x + 2, y + 2, x + 10, y + 9, COLOR_RED);
        _tft.drawLine(x + 10, y + 2, x + 2, y + 9, COLOR_RED);
        return;
    }

    uint16_t barColor = COLOR_GREEN;
    if (rssi < -80) {
        barColor = COLOR_RED;
    } else if (rssi < -70) {
        barColor = COLOR_YELLOW;
    }

    int bars = 1;
    if (rssi >= -60) bars = 4;
    else if (rssi >= -70) bars = 3;
    else if (rssi >= -80) bars = 2;

    for (int b = 0; b < 4; b++) {
        int barH = (b + 1) * 2;
        uint16_t c = (b < bars) ? barColor : COLOR_DARKGREY;
        _tft.fillRect(x + (b * 3), y + (9 - barH), 2, barH, c);
    }
}

void DisplayUI::updateWifiStatus(bool connected, int rssi, const IPAddress& ip) {
    if (connected != _lastWifiState || abs(rssi - _lastRssi) > 4) {
        drawWifiIndicator(110, 3, connected, rssi);
        _lastWifiState = connected;
        _lastRssi = rssi;
    }
}

void DisplayUI::updateStatusBar(const char* message) {
    _tft.fillRect(0, 150, 128, 10, COLOR_BLACK);
    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_LIGHTGREY, COLOR_BLACK);
    _tft.setCursor(2, 151);
    char statBuf[22];
    snprintf(statBuf, sizeof(statBuf), "%-21.21s", message);
    _tft.print(statBuf);
}

void DisplayUI::drawWeatherIcon(int x, int y, WeatherIconType type, bool small) {
    int r = small ? 4 : 7;

    switch (type) {
        case ICON_SUNNY:
            _tft.fillCircle(x, y, r, COLOR_YELLOW);
            for (int i = 0; i < 8; i++) {
                float angle = i * (3.14159f / 4.0f);
                int x1 = x + (int)((r + 2) * cos(angle));
                int y1 = y + (int)((r + 2) * sin(angle));
                int x2 = x + (int)((r + (small ? 3 : 4)) * cos(angle));
                int y2 = y + (int)((r + (small ? 3 : 4)) * sin(angle));
                _tft.drawLine(x1, y1, x2, y2, COLOR_ORANGE);
            }
            break;

        case ICON_PARTLY_CLOUDY:
            // Sol atrás
            _tft.fillCircle(x - (small ? 3 : 5), y - (small ? 3 : 4), small ? 3 : 5, COLOR_YELLOW);
            // Nube delante
            _tft.fillCircle(x, y + (small ? 1 : 2), small ? 3 : 5, COLOR_WHITE);
            _tft.fillCircle(x + (small ? 4 : 6), y + (small ? 2 : 2), small ? 3 : 4, COLOR_LIGHTGREY);
            _tft.fillRect(x - (small ? 3 : 4), y + (small ? 2 : 3), small ? 9 : 13, small ? 3 : 4, COLOR_WHITE);
            break;

        case ICON_CLOUDY:
            _tft.fillCircle(x - (small ? 3 : 4), y + 1, small ? 3 : 5, COLOR_LIGHTGREY);
            _tft.fillCircle(x + 1, y - (small ? 2 : 3), small ? 4 : 6, COLOR_WHITE);
            _tft.fillCircle(x + (small ? 4 : 5), y + 1, small ? 3 : 4, COLOR_LIGHTGREY);
            _tft.fillRect(x - (small ? 3 : 4), y + (small ? 1 : 2), small ? 9 : 12, small ? 3 : 4, COLOR_WHITE);
            break;

        case ICON_RAIN:
            // Nube
            _tft.fillCircle(x - (small ? 3 : 4), y - 2, small ? 3 : 4, COLOR_DARKGREY);
            _tft.fillCircle(x + 1, y - (small ? 3 : 4), small ? 4 : 5, COLOR_LIGHTGREY);
            _tft.fillCircle(x + (small ? 4 : 4), y - 2, small ? 3 : 4, COLOR_DARKGREY);
            _tft.fillRect(x - (small ? 3 : 4), y - 1, small ? 8 : 10, small ? 3 : 4, COLOR_LIGHTGREY);
            // Gotas
            _tft.drawLine(x - 3, y + 3, x - 4, y + (small ? 6 : 8), COLOR_CYAN);
            _tft.drawLine(x + 1, y + 3, x, y + (small ? 6 : 8), COLOR_CYAN);
            _tft.drawLine(x + 4, y + 3, x + 3, y + (small ? 6 : 8), COLOR_CYAN);
            break;

        case ICON_SNOW:
            _tft.fillCircle(x, y - 2, small ? 3 : 5, COLOR_LIGHTGREY);
            _tft.drawPixel(x - 3, y + 3, COLOR_WHITE);
            _tft.drawPixel(x, y + 5, COLOR_WHITE);
            _tft.drawPixel(x + 3, y + 3, COLOR_WHITE);
            break;

        case ICON_THUNDERSTORM:
            _tft.fillCircle(x - (small ? 2 : 3), y - 2, small ? 3 : 4, COLOR_DARKGREY);
            _tft.fillCircle(x + (small ? 2 : 3), y - 2, small ? 3 : 4, COLOR_DARKGREY);
            // Rayo
            _tft.drawLine(x, y, x - 2, y + 3, COLOR_YELLOW);
            _tft.drawLine(x - 2, y + 3, x + 1, y + 3, COLOR_YELLOW);
            _tft.drawLine(x + 1, y + 3, x - 1, y + 7, COLOR_YELLOW);
            break;

        default:
            _tft.drawCircle(x, y, r, COLOR_WHITE);
            break;
    }
}
