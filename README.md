# LilyGo EPD 4.7" Weather Display - Version Touch

![Weather Station Display](LilyGo-EPD-4-7-WeatherAPI-Display-Touch.jpeg)

**[English](README_EN.md)** | **[Francais](README_FR.md)**

Estacion meteorologica para pantalla e-paper LilyGo T5 4.7" con navegacion tactil completa.

## Caracteristicas

- **Navegacion tactil** - 11+ pantallas navegables por touch
- **Clima actual** - Temperatura, humedad, presion, viento, Indice UV, Calidad del Aire
- **Pronostico 3 dias** - Predicciones horarias y diarias
- **Graficas del clima** - Tendencias de temperatura, presion, humedad, precipitacion
- **Historial de datos** - Hasta 1 ano de datos grabados (con tarjeta SD)
- **Fase lunar** - Fase lunar actual con icono
- **Amanecer/atardecer** - Horarios solares diarios
- **Calendario** - Vistas mensual y anual
- **Narrativa del clima** - Descripciones generadas por IA en el idioma seleccionado (Groq/Llama)
- **Frase del dia** - Citas inspiracionales diarias
- **Reloj mundial** - Mapa de zonas horarias
- **Configuracion web** - Configura via WiFi AP, sin recompilar
- **Configuracion Bluetooth** - App Android para configuracion BLE
- **Multi-idioma** - Espanol, Ingles, Frances
- **Multi-WiFi** - Se conecta a la red mas fuerte de hasta 3 configuradas
- **Deep sleep** - Operacion eficiente en bateria con intervalo configurable
- **Soporte tarjeta SD** - Almacenamiento extendido de historial (~52,000 lecturas)

## Hardware

**Requerido:** LilyGo T5 4.7" S3 Touch (ESP32-S3, e-paper 960x540, touch GT911)

Opcional: Tarjeta MicroSD para historial extendido

## Inicio Rapido

### 1. Instalar Firmware

#### Opcion Recomendada: Web Flasher (sin instalar nada)

> **La forma mas facil de instalar.** Solo necesitas un navegador y cable USB.

[![Instalar Firmware](https://img.shields.io/badge/🚀_INSTALAR_FIRMWARE-blue?style=for-the-badge&logo=esphome)](https://xe1e.github.io/LilyGo-EPD-4-7-WeatherAPI-Display-Touch/)

1. Conectar dispositivo via cable USB-C
2. Abrir el link en **Chrome, Edge u Opera**
3. Click en "Instalar Firmware" y seleccionar puerto COM
4. Esperar ~1 minuto a que termine

#### Opcion Alternativa: Arduino IDE (para desarrolladores)

<details>
<summary>Click para ver configuracion Arduino IDE</summary>

**Configuracion Arduino IDE:**
| Configuracion | Valor |
|---------------|-------|
| Board | ESP32S3 Dev Module |
| USB CDC On Boot | Enable |
| USB DFU On Boot | Disable |
| Flash Size | 16MB (128Mb) |
| Flash Mode | QIO 80MHz |
| Partition Scheme | 16M Flash (3M APP/9.9MB FATFS) |
| PSRAM | OPI PSRAM |
| Upload Mode | UART0/Hardware CDC |
| USB Mode | Hardware CDC and JTAG |

**Librerias Requeridas:**
- Board Manager: esp32 by Espressif Systems 2.0.17
- EPD47-master: https://github.com/DFRobotdl/EPD47/archive/refs/heads/master.zip
- ArduinoJson: by Benoit Blanchon 6.19.0

</details>

### Actualizaciones de Firmware (OTA)

El dispositivo soporta actualizaciones inalambricas:

| Metodo | URL / Como usar |
|--------|-----------------|
| **Web OTA** | `http://[IP_DISPOSITIVO]/ota` - Subir .bin desde navegador |
| **Arduino OTA** | Seleccionar puerto de red "WeatherStation" en Arduino IDE |
| **Web Flasher** | [xe1e.github.io/LilyGo-EPD-4-7-WeatherAPI-Display-Touch](https://xe1e.github.io/LilyGo-EPD-4-7-WeatherAPI-Display-Touch/) |
| **Releases** | [Descargar archivos .bin](https://github.com/XE1E/LilyGo-EPD-4-7-WeatherAPI-Display-Touch/releases) |

### 2. Configuracion Inicial

En el primer encendido, el dispositivo detecta automaticamente que no hay configuracion y entra en **modo configuracion inicial** (sin limite de tiempo):

1. Conectarse a red WiFi: `WeatherStation-Setup`
2. Password: `weather123`
3. Abrir navegador: `http://192.168.4.1`
4. **Probar WiFi** - Boton para verificar que la red existe
5. **Probar API** - Boton para validar tu API key antes de guardar
6. Ingresar configuracion (API keys, ubicacion, etc.)
7. Click en Guardar - el dispositivo reinicia automaticamente en 5 segundos

**Modos de configuracion:**
| Modo | Cuando ocurre | Timeout |
|------|---------------|---------|
| Configuracion Inicial | Primer encendido, sin config | Sin limite |
| Modo Recuperacion | Falla conexion WiFi | 5 minutos |
| Modo Forzado | `FORCE_AP_MODE=true` | Sin limite |

### 3. Operacion Normal

Despues de configurar, el dispositivo:
1. Se conecta a WiFi
2. Obtiene clima de WeatherAPI.com (una sola llamada HTTPS)
3. Muestra el clima en pantalla
4. Permite 30 segundos de navegacion tactil
5. Entra en deep sleep (intervalo configurable)
6. Despierta y repite

## Navegacion Tactil

| Zona de Pantalla | Accion |
|------------------|--------|
| Icono grande del clima (centro) | Narrativa del Clima (IA) |
| Area bateria/WiFi (arriba-derecha) | Info del Sistema |
| Area luna/sol (arriba-izquierda) | Calendario |
| Temperatura/Clima (superior) | Condiciones Actuales |
| Pronostico por hora (medio) | Pronostico Extendido |
| Mitad izquierda graficas | Historial del Clima |
| Mitad derecha graficas | Tendencias del Pronostico |

## Opciones de Configuracion

| Campo | Descripcion | Ejemplo |
|-------|-------------|---------|
| WiFi (hasta 3) | SSID y password de red | MiWiFi / password123 |
| WeatherAPI Key | API key de WeatherAPI.com | abc123... |
| Groq API Key | Para narrativa IA (gratis) | gsk_... |
| Ciudad | Nombre de ciudad para mostrar | Ciudad de Mexico |
| Latitud | Latitud de ubicacion | 19.4326 |
| Longitud | Longitud de ubicacion | -99.1332 |
| Zona Horaria | String POSIX de zona horaria | CST6 |
| Intervalo | Minutos entre actualizaciones | 30 |
| Idioma | Idioma de interfaz | ES / EN / FR |
| Unidades | Metrico o Imperial | M / I |
| Estilo narrativa | Estilo de texto IA | Radio, Formal, Poetico... |

### Aplicacion de Cambios

| Aplica inmediatamente | Requiere reinicio |
|-----------------------|-------------------|
| Idioma | Credenciales WiFi |
| Unidades (C/F) | API Keys |
| Intervalo de actualizacion | Ubicacion/Coordenadas |
| Horario de actividad | Zona horaria |
| Estilo de narrativa | |

## API Keys

### WeatherAPI.com (requerido)
1. Registrarse en https://www.weatherapi.com/
2. Ir al Dashboard
3. Copiar tu API key

### Groq (opcional, para Narrativa del Clima)
1. Registrarse en https://console.groq.com/
2. Crear una API key
3. Tier gratuito: 30 solicitudes/minuto

## Pantallas

| # | Pantalla | Acceso |
|---|----------|--------|
| 1 | Clima Principal | Por defecto |
| 2 | Condiciones Actuales | Tocar area superior |
| 3 | Pronostico Extendido | Tocar iconos por hora |
| 4 | Tendencias del Clima | Tocar graficas derecha |
| 5 | Historial del Clima | Tocar graficas izquierda |
| 6 | Info del Sistema (5 paginas) | Tocar bateria/WiFi |
| 7 | Calidad del Aire | Desde Condiciones Actuales |
| 8 | Calendario (Mensual/Anual) | Tocar area luna/sol |
| 9 | Narrativa del Clima | Tocar icono grande |
| 10 | Frase del Dia | Oculto: Info > Creditos > ... |
| 11 | Reloj Mundial | Navegacion oculta |

## Solucion de Problemas

### Falla la subida
1. Presionar y mantener boton BOOT
2. Mientras mantiene BOOT, presionar RST
3. Soltar RST, luego soltar BOOT
4. La subida deberia funcionar

### Touch no responde
- Limpiar superficie de pantalla
- Verificar deteccion GT911 en Monitor Serial
- Tocar firmemente por al menos 50ms

### Sin datos del clima
- Verificar que tu API key de WeatherAPI.com sea valida
- Error 403 = API key invalida
- Verificar latitud/longitud correctas
- Revisar credenciales WiFi

### Fantasmas en pantalla
- Usar boton "Limpiar Pantalla" en Info del Sistema
- O reiniciar el dispositivo

## Archivos

| Archivo | Descripcion |
|---------|-------------|
| `LilyGo-EPD-4-7-WeatherAPI-Display-Touch.ino` | Sketch principal |
| `wifi_manager.h` | Modo AP y servidor web |
| `owm_credentials.h` | Configuracion por defecto |
| `lang.h` | Strings multi-idioma |
| `touch_handler.h` | Navegacion tactil |
| `weather_narrative.h` | Descripciones IA del clima |
| `weather_history.h` | Sistema de almacenamiento |
| `calendar.h` | Vistas de calendario |
| `quote_screen.h` | Frase del dia |
| `forecast_record.h` | Estructura de datos |
| `opensans*.h` | Archivos de fuentes |

## Detalles Tecnicos

- **Pantalla:** 960x540 pixeles, escala de grises 4-bit
- **Touch:** Controlador capacitivo GT911
- **MCU:** ESP32-S3 con 8MB PSRAM
- **Consumo:** Deep sleep entre actualizaciones (~10uA)
- **Almacenamiento:** NVS + FFat (~7 dias) + tarjeta SD (~1 ano)
- **Fuente de despertar:** Timer

## Documentacion

Ver manuales detallados en tres idiomas:
- [MANUAL.md](MANUAL.md) - Espanol
- [MANUAL_EN.md](MANUAL_EN.md) - English
- [MANUAL_FR.md](MANUAL_FR.md) - Francais

## Licencia

Basado en trabajo original de David Bird - Ver Licence.txt

## Creditos

- Codigo original por David Bird 2021
- Port ESP32 por Xinyuan-LilyGO
- Modificado por Stefan Maetschke 2025
- Version touch con extensiones por XE1E 2026
- Libreria EPD47 por Vroland/DFRobot
- Datos del clima de WeatherAPI.com
- Narrativa IA por Groq/Llama

---

73 de XE1E
