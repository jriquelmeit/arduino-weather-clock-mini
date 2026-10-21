#include "weather_service.h"
#include "time_service.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

const char* getWeatherDescription(int code) {
    switch (code) {
        case 0:  return "Despejado";
        case 1:  return "Mayormente claro";
        case 2:  return "Parcialmente nublado";
        case 3:  return "Nublado";
        case 45:
        case 48: return "Niebla";
        case 51: return "Llovizna ligera";
        case 53: return "Llovizna moderada";
        case 55: return "Llovizna densa";
        case 56:
        case 57: return "Llovizna helada";
        case 61: return "Lluvia ligera";
        case 63: return "Lluvia moderada";
        case 65: return "Lluvia fuerte";
        case 66:
        case 67: return "Lluvia helada";
        case 71: return "Nieve ligera";
        case 73: return "Nieve moderada";
        case 75: return "Nieve fuerte";
        case 77: return "Granizo";
        case 80: return "Chubascos suaves";
        case 81: return "Chubascos mod.";
        case 82: return "Chubascos fuertes";
        case 85:
        case 86: return "Nieve chubasco";
        case 95: return "Tormenta";
        case 96:
        case 99: return "Tormenta granizo";
        default: return "Variable";
    }
}

WeatherIconType getWeatherIconType(int code) {
    switch (code) {
        case 0:
        case 1:
            return ICON_SUNNY;
        case 2:
            return ICON_PARTLY_CLOUDY;
        case 3:
        case 45:
        case 48:
            return ICON_CLOUDY;
        case 51:
        case 53:
        case 55:
        case 56:
        case 57:
        case 61:
        case 63:
        case 65:
        case 66:
        case 67:
        case 80:
        case 81:
        case 82:
            return ICON_RAIN;
        case 71:
        case 73:
        case 75:
        case 77:
        case 85:
        case 86:
            return ICON_SNOW;
        case 95:
        case 96:
        case 99:
            return ICON_THUNDERSTORM;
        default:
            return ICON_PARTLY_CLOUDY;
    }
}

WeatherService::WeatherService() {}

bool WeatherService::fetchWeather(const char* lat, const char* lon, WeatherData& outData) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(F("[Clima] Error: Wi-Fi desconectado"));
        return false;
    }

    WiFiClient client;
    HTTPClient http;

    http.setTimeout(6000);

    String url = "http://api.open-meteo.com/v1/forecast?latitude=";
    url += lat;
    url += "&longitude=";
    url += lon;
    url += "&current=temperature_2m,relative_humidity_2m,weather_code";
    url += "&daily=weather_code,temperature_2m_max,temperature_2m_min";
    url += "&timezone=auto&forecast_days=3";

    Serial.print(F("[Clima] Solicitando API Open-Meteo: "));
    Serial.println(url);

    if (!http.begin(client, url)) {
        Serial.println(F("[Clima] Error: no se pudo iniciar conexion HTTP"));
        return false;
    }

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("[Clima] Error HTTP: %d (%s)\n", httpCode, http.errorToString(httpCode).c_str());
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    Serial.printf("[Clima] Respuesta recibida (%u bytes)\n", payload.length());

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
        Serial.printf("[Clima] Error al parsear JSON: %s\n", error.c_str());
        return false;
    }

    // Parsear clima actual
    outData.current.temperature = doc["current"]["temperature_2m"] | 0.0f;
    outData.current.humidity = doc["current"]["relative_humidity_2m"] | 0;
    outData.current.weatherCode = doc["current"]["weather_code"] | 0;
    snprintf(outData.current.conditionText, sizeof(outData.current.conditionText), "%s",
             getWeatherDescription(outData.current.weatherCode));
    outData.current.iconType = getWeatherIconType(outData.current.weatherCode);

    // Parsear pronóstico de hoy (índice 0) y mañana (índice 1)
    for (int i = 0; i < 2; i++) {
        int dayIndex = i; // 0 = Hoy, 1 = Mañana
        const char* dStr = doc["daily"]["time"][dayIndex] | "";
        snprintf(outData.forecast[i].date, sizeof(outData.forecast[i].date), "%s", dStr);
        getSpanishDayFromIsoDate(dStr, outData.forecast[i].dayName, sizeof(outData.forecast[i].dayName));

        if (i == 0) {
            // Para el día actual, formatear como "HOY DD" (ej: "HOY 17")
            int y = 0, m = 0, d = 0;
            if (sscanf(dStr, "%d-%d-%d", &y, &m, &d) == 3) {
                snprintf(outData.forecast[0].shortDayName, sizeof(outData.forecast[0].shortDayName), "HOY %02d", d);
            } else {
                snprintf(outData.forecast[0].shortDayName, sizeof(outData.forecast[0].shortDayName), "HOY");
            }
        } else {
            // Para el día siguiente, formatear como "DIA DD" (ej: "DOM 18")
            getSpanishShortDayFromIsoDate(dStr, outData.forecast[i].shortDayName, sizeof(outData.forecast[i].shortDayName));
        }

        outData.forecast[i].tempMax = doc["daily"]["temperature_2m_max"][dayIndex] | 0.0f;
        outData.forecast[i].tempMin = doc["daily"]["temperature_2m_min"][dayIndex] | 0.0f;
        outData.forecast[i].weatherCode = doc["daily"]["weather_code"][dayIndex] | 0;
        snprintf(outData.forecast[i].conditionText, sizeof(outData.forecast[i].conditionText), "%s",
                 getWeatherDescription(outData.forecast[i].weatherCode));
        outData.forecast[i].iconType = getWeatherIconType(outData.forecast[i].weatherCode);
    }

    outData.valid = true;
    outData.lastFetchTime = millis();

    Serial.println(F("[Clima] Datos meteorologicos actualizados con exito."));
    Serial.printf("  Actual: %.1f C, Hum: %d%%, Cond: %s\n",
                  outData.current.temperature, outData.current.humidity, outData.current.conditionText);
    Serial.printf("  Hoy (%s): Min: %.1f C, Max: %.1f C, %s\n",
                  outData.forecast[0].shortDayName, outData.forecast[0].tempMin, outData.forecast[0].tempMax, outData.forecast[0].conditionText);
    Serial.printf("  Manana (%s): Min: %.1f C, Max: %.1f C, %s\n",
                  outData.forecast[1].shortDayName, outData.forecast[1].tempMin, outData.forecast[1].tempMax, outData.forecast[1].conditionText);

    return true;
}
