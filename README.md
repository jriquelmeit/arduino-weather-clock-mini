# ESP8266 Weather Clock Mini (1.8" / 1.77" TFT SPI 128*160)

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Core_6-orange?logo=platformio)](https://platformio.org/)
[![ESP8266](https://img.shields.io/badge/Board-NodeMCU_V3_(ESP8266)-blue?logo=espressif)](https://www.espressif.com/)
[![Display](https://img.shields.io/badge/Display-ST7735S_/_ST7735B_SPI-red)](https://www.adafruit.com/)
[![Framework](https://img.shields.io/badge/Framework-Arduino-00979D?logo=arduino)](https://www.arduino.cc/)
[![API](https://img.shields.io/badge/Weather_API-Open--Meteo-brightgreen)](https://open-meteo.com/)

Versión compacta del reloj meteorológico de escritorio con sincronización Wi-Fi para la placa **Lolin NodeMCU V3 (ESP8266)** y pantallas gráficas **1.8" o 1.77" TFT SPI 128x160** (controladores **ST7735S** o **ST7735B**) en orientación **vertical (128x160 px)**.

Sincroniza la hora automáticamente mediante NTP con reglas POSIX de cambio estacional de horario (verano/invierno) y obtiene el clima actual más el pronóstico de 2 días de la API de **Open-Meteo**, con interfaz en **español** y redibujado *flicker-free*.

---

## Distribución de la Pantalla (Vertical 128 x 160 px)

La interfaz se divide verticalmente en 5 secciones optimizadas:

```text
 0px +------------------------------+
     | Santiago               [WiFi]|  -> Cabecera: Ciudad + Estado Wi-Fi / RSSI
15px +------------------------------+
     |                              |
     |           14:35              |  -> Reloj Digital (Tamaño 3)
     |        SAB 17 OCT            |  -> Fecha Compacta en Español (Tamaño 1)
     |                              |
63px +------------------------------+
     | CLIMA               62% Hum  |
     |  [ICON]    21.5 C            |  -> Clima Actual: Icono + Temp + Condición
     |  14x14     Despejado         |
112px+------------------------------+
     |  DOM 18         LUN 19       |
     | [i] 12/24      [i] 14/26     |  -> Pronóstico 2 Días (2 columnas compactas)
148px+------------------------------+
     | 14:35 | 192.168.1.45         |  -> Barra de estado inferior
160px+------------------------------+
```

---

## Diagrama de Conexiones

> [!WARNING]
> **¡Atención a la serigrafía de tu pantalla!** Existen dos disposiciones de pines muy comunes en módulos de 1.8" y 1.77":
> - **Variante A (Ej. 1.8" V1.1):** Empieza en **GND** y termina en **BLK**.
> - **Variante B (Ej. 1.77" ST7735S):** Empieza en **VCC** y termina en **LED**.
> 
> Conectar los cables según la tabla incorrecta invertirá VCC y GND. Revisa siempre la serigrafía impresa en el reverso de tu módulo.

---

### Opción A: Pantallas que inician en GND y terminan en BLK (Ej. 1.8" V1.1)

| Pin Pantalla (Variante A) | Pin NodeMCU (Serigrafía) | GPIO ESP8266 | Función |
| :--- | :--- | :--- | :--- |
| **GND** (Pin 1) | **GND** | - | Tierra común |
| **VCC** (Pin 2) | **VIN (5V)** ó **3V3** | - | Alimentación |
| **SCL** (Pin 3) | **D5** | GPIO14 | Reloj SPI (HSPI SCK) |
| **SDA** (Pin 4) | **D7** | GPIO13 | Datos SPI (HSPI MOSI) |
| **RES** (Pin 5) | **D4** | GPIO2 | Reset del display |
| **DC**  (Pin 6) | **D3** | GPIO0 | Selección Datos / Comando |
| **CS**  (Pin 7) | **D8** | GPIO15 | Chip Select TFT |
| **BLK** (Pin 8) | **3V3** | - | Retroiluminación (Backlight) |

---

### Opción B: Pantallas que inician en VCC y terminan en LED (Ej. 1.77" ST7735S)

| Pin Pantalla (Variante B) | Pin NodeMCU (Serigrafía) | GPIO ESP8266 | Función |
| :--- | :--- | :--- | :--- |
| **VCC** (Pin 1) | **VIN (5V)** ó **3V3** | - | Alimentación |
| **GND** (Pin 2) | **GND** | - | Tierra común |
| **CS**  (Pin 3) | **D8** | GPIO15 | Chip Select TFT |
| **RESET / RES** (Pin 4) | **D4** | GPIO2 | Reset del display |
| **A0 / DC** (Pin 5) | **D3** | GPIO0 | Selección Datos / Comando |
| **SDA / MOSI** (Pin 6) | **D7** | GPIO13 | Datos SPI (HSPI MOSI) |
| **SCK / SCL** (Pin 7) | **D5** | GPIO14 | Reloj SPI (HSPI SCK) |
| **LED** (Pin 8) | **3V3** | - | Retroiluminación (Backlight) |

---

```text
Conexión de Señales al NodeMCU (válida para ambas variantes):
       +----------------------------+
       |   Lolin NodeMCU V3         |
       |      (ESP8266)             |
       +----------------------------+
       |   VIN (5V) / 3V3           | -----> VCC Pantalla
       |   GND                      | -----> GND Pantalla
       |   D8 (GPIO15)              | -----> CS
       |   D4 (GPIO2)               | -----> RESET / RES
       |   D3 (GPIO0)               | -----> DC / RS / A0
       |   D7 (GPIO13 - HSPI MOSI)  | -----> SDA / MOSI
       |   D5 (GPIO14 - HSPI CLK)   | -----> SCK / SCL
       |   3V3                      | -----> LED / BLK (Backlight)
       +----------------------------+
```

> [!NOTE]
> Si la pantalla tiene lector de tarjetas SD en el reverso (`SD_CS`, `SD_MOSI`, etc.), no es necesario conectarlo para este proyecto.

---

## Configuración y Puesta en Marcha

### 1. Configurar Credenciales y Ubicación

Copia la plantilla de configuración:

```bash
cp include/config.h.example include/config.h
```

Edita `include/config.h`:

```cpp
// Red Wi-Fi (solo 2.4 GHz)
#define WIFI_SSID       "Tu_Red_WiFi"
#define WIFI_PASSWORD   "Tu_Contraseña"

// Orientación (0: vertical estándar, 2: vertical invertida 180°)
#define TFT_ROTATION    0

// Controlador del panel:
// - TFT_ST7735S_R: Para módulo 1.77" o 1.8" con chip ST7735S / ST7735R (usa initR)
// - TFT_ST7735B:   Para módulo 1.8" con chip ST7735B (usa initB)
#define TFT_DRIVER_TYPE TFT_ST7735S_R

// Tipo de inicialización (para ST7735S):
// INITR_BLACKTAB (estándar), INITR_REDTAB, INITR_GREENTAB
#define ST7735_INIT_TAB INITR_BLACKTAB

// Inversión de colores (false: normal, true: si los colores se ven en negativo)
#define TFT_INVERT_COLORS false

// Ubicación y regla POSIX horaria
#define LOCATION_NAME   "Santiago"
#define LOCATION_LAT    "-33.4489"
#define LOCATION_LON    "-70.6693"
#define TIMEZONE_RULE   "CLT4CLST,M9.1.0/0,M4.1.0/0"
```

---

## Compilación y Carga

### Desde la Terminal con PlatformIO

1. **Compilar el proyecto:**
   ```bash
   pio run
   ```

2. **Subir el firmware al NodeMCU:**
   ```bash
   pio run -t upload
   ```

3. **Abrir monitor serie (115200 baudios):**
   ```bash
   pio device monitor
   ```

---

## Estructura del Proyecto

```text
arduino-weather-clock-mini/
├── platformio.ini              # Dependencias y configuración PlatformIO
├── .gitignore                  # Exclusiones de Git y credenciales
├── include/
│   ├── config.h.example        # Plantilla de configuración
│   ├── config.h                # Credenciales y pines locales (ignorado por Git)
│   ├── time_service.h          # SNTP y calendario en español
│   ├── weather_service.h       # Modelo de datos meteorológicos Open-Meteo
│   └── display_ui.h            # Interfaz gráfica vertical para ST7735
├── src/
│   ├── main.cpp                # Setup, timers y loop no bloqueante
│   ├── time_service.cpp        # Manejo de hora y días en español
│   ├── weather_service.cpp     # Cliente HTTP y parseo JSON
│   └── display_ui.cpp          # Renderizado de reloj, iconos vectoriales y tarjetas
└── README.md
```

---

## Solución de Problemas Comunes

- **Pantalla blanca o sin respuesta en chip ST7735B:**
  - Si tu pantalla de 1.8" tiene el chip ST7735B (suele indicar alimentación de 3V), asegúrate de configurar `#define TFT_DRIVER_TYPE TFT_ST7735B` en `include/config.h` para que ejecute la secuencia `initB()`, y conecta su VCC al pin **3V3**.
- **Colores invertidos o bordes con ruido / líneas aleatorias en pantalla (ST7735S):**
  - Si los colores se ven en negativo, activa `#define TFT_INVERT_COLORS true`.
  - Si observas 1-2 píxeles de desfase en el borde superior o izquierdo, cambia `#define ST7735_INIT_TAB INITR_REDTAB` (o `INITR_BLACKTAB`).
- **Pantalla blanca sin contenido:**
  - Verifica que `SCK`/`SCL` esté conectado al pin **D5** y `SDA`/`MOSI` al pin **D7**.
  - Confirma que `CS` esté en **D8** y `DC`/`A0` en **D3**.
- **La placa no arranca al enchufarla:**
  - El pin `D3 (GPIO0)` y `D8 (GPIO15)` determinan el modo de arranque del chip ESP8266. Si la pantalla demanda mucha corriente al inicio, conecta el pin `VCC` a `VIN (5V)` en vez de `3V3` (siempre que la placa cuente con regulador LDO a 3.3V).
