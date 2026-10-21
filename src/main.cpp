#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "config.h"
#include "time_service.h"
#include "weather_service.h"
#include "display_ui.h"

// Instancias principales
static DisplayUI ui(TFT_CS, TFT_DC, TFT_RST);
static WeatherService weatherService;
static WeatherData weatherData;

// Temporizadores
static unsigned long lastClockTick = 0;
static unsigned long lastWeatherAttempt = 0;
static unsigned long lastWifiStatusCheck = 0;
static bool baseLayoutDrawn = false;
static bool initialWeatherFetched = false;

void connectWiFi() {
    Serial.println(F("\n========================================"));
    Serial.printf("Conectando a red Wi-Fi: %s\n", WIFI_SSID);
    Serial.println(F("========================================"));

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    uint8_t dots = 0;
    unsigned long startAttempt = millis();

    while (WiFi.status() != WL_CONNECTED && (millis() - startAttempt < 20000)) {
        delay(400);
        Serial.print(F("."));
        ui.updateBootProgress("Conectando Wi-Fi", dots++);
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println(F("\n[Wi-Fi] Conectado con exito!"));
        Serial.printf("  IP Asignada: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("  Senal (RSSI): %d dBm\n", WiFi.RSSI());
        ui.updateBootProgress("Wi-Fi Conectado", 0);
    } else {
        Serial.println(F("\n[Wi-Fi] Tiempo de espera agotado. Continuando en segundo plano..."));
        ui.updateBootProgress("Sin Wi-Fi", 0);
    }
}

void setup() {
    Serial.begin(115200);
    delay(200);

    Serial.println(F("\n=============================================="));
    Serial.println(F("   ESP8266 Weather Clock Mini - TFT 1.8 ST7735"));
    Serial.println(F("=============================================="));

    // 1. Inicializar pantalla TFT (ST7735S o ST7735B)
    ui.begin(TFT_ROTATION, TFT_DRIVER_TYPE, ST7735_INIT_TAB, TFT_INVERT_COLORS);
    ui.showBootScreen(WIFI_SSID);

    // 2. Conectar a red Wi-Fi
    connectWiFi();

    // 3. Inicializar servicio de tiempo (NTP con regla POSIX de zona horaria)
    Serial.println(F("[NTP] Configurando sincronizacion horaria..."));
    initTimeService(TIMEZONE_RULE, NTP_SERVER_1, NTP_SERVER_2);

    // Esperar sincronización inicial de tiempo (máx 6 seg)
    unsigned long ntpWait = millis();
    uint8_t step = 0;
    while (!isTimeSynchronized() && (millis() - ntpWait < 6000)) {
        delay(300);
        ui.updateBootProgress("Sincronizando NTP", step++);
    }

    if (isTimeSynchronized()) {
        Serial.println(F("[NTP] Hora sincronizada correctamente."));
    } else {
        Serial.println(F("[NTP] Esperando respuesta de servidor de tiempo..."));
    }

    // 4. Dibujar plantilla base en pantalla (CUSTOM_DISPLAY_NAME o LOCATION_NAME por defecto)
#ifdef CUSTOM_DISPLAY_NAME
    const char* headerTitle = (strlen(CUSTOM_DISPLAY_NAME) > 0) ? CUSTOM_DISPLAY_NAME : LOCATION_NAME;
    ui.drawBaseLayout(headerTitle);
#else
    ui.drawBaseLayout(LOCATION_NAME);
#endif
    baseLayoutDrawn = true;

    // 5. Carga inicial del pronóstico meteorológico si hay Wi-Fi
    if (WiFi.status() == WL_CONNECTED) {
        ui.updateStatusBar("Cargando clima...");
        if (weatherService.fetchWeather(LOCATION_LAT, LOCATION_LON, weatherData)) {
            ui.updateWeather(weatherData);
            initialWeatherFetched = true;
            ui.updateStatusBar("Clima actualizado");
        }
    }
}

void loop() {
    unsigned long now = millis();
    bool isConnected = (WiFi.status() == WL_CONNECTED);

    // 1. Refresco del Reloj y Fecha (cada 1 segundo)
    if (now - lastClockTick >= CLOCK_REFRESH_INTERVAL_MS) {
        lastClockTick = now;

        DateTimeInfo dt;
        if (getCurrentDateTime(dt)) {
            ui.updateTime(dt);
            ui.updateDate(dt);
        }
    }

    // 2. Comprobación y actualización de estado Wi-Fi (cada 4 segundos)
    if (now - lastWifiStatusCheck >= 4000) {
        lastWifiStatusCheck = now;
        ui.updateWifiStatus(isConnected, WiFi.RSSI(), WiFi.localIP());

        // Si se reconecta y aún no teníamos clima, forzar consulta
        if (isConnected && !weatherData.valid) {
            lastWeatherAttempt = 0;
        }
    }

    // 3. Refresco del Clima (cada 15 minutos o reintento de 1 min si falló)
    unsigned long weatherInterval = weatherData.valid ? WEATHER_REFRESH_INTERVAL_MS : WEATHER_RETRY_INTERVAL_MS;

    if (isConnected && (now - lastWeatherAttempt >= weatherInterval || !initialWeatherFetched)) {
        lastWeatherAttempt = now;

        Serial.println(F("[Loop] Actualizando datos de clima..."));
        if (weatherService.fetchWeather(LOCATION_LAT, LOCATION_LON, weatherData)) {
            ui.updateWeather(weatherData);
            initialWeatherFetched = true;

            DateTimeInfo dt;
            char statusMsg[24];
            if (getCurrentDateTime(dt)) {
                snprintf(statusMsg, sizeof(statusMsg), "%s | %s",
                         dt.shortTimeStr,
                         WiFi.localIP().toString().c_str());
            } else {
                snprintf(statusMsg, sizeof(statusMsg), "%s",
                         WiFi.localIP().toString().c_str());
            }
            ui.updateStatusBar(statusMsg);
        } else {
            ui.updateStatusBar("Error clima");
        }
    }

    // Permitir ejecución suave de las tareas del ESP8266 (Wi-Fi, TCP, watchdog)
    delay(20);
}
