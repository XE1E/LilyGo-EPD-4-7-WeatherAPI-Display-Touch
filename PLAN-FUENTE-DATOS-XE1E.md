# Plan: alimentar el e-paper con datos de la estación XE1E

Sustituir WeatherAPI.com por el servidor propio (`clima.xe1e.net`) **conservando
intacto todo lo demás**: las 11+ pantallas, el dibujado, el touch, el deep sleep y la
configuración web.

## Por qué es barato

La frontera real del firmware **no es el JSON, es la struct**. Todo el dibujado lee de
`WxConditions[0]` y `WxForecast[]` (`forecast_record.h`); la única función que toca JSON
es `DecodeWeatherAPI()` (líneas 1044‑1249). Cambiar la fuente de datos es una
intervención en ~200 líneas que no roza lo visual.

Además existe un precedente propio ya en producción: `receiver/app/services/svitrix.py`
del servidor construye JSON **con forma WeatherAPI** desde el dato real de la estación
para el reloj Ulanzi, incluida la condición derivada por índice de claridad, el `is_day`
por elevación solar y la calidad del aire. Es reusable casi tal cual.

## Arquitectura elegida

**Forma WeatherAPI como base del payload, más un bloque propio al lado.**

- El servidor emite `location` + `current` + `forecast.forecastday[]` con el esquema de
  WeatherAPI (reusa `svitrix.py`, trabajo ya hecho) **y** un bloque `xe1e{}` con lo que
  WeatherAPI no puede dar.
- El firmware conserva `DecodeWeatherAPI()` como camino de respaldo — basta cambiar la
  URL para volver a WeatherAPI.com si el VPS cae.
- Es el mismo patrón ya validado en el reloj (`solar_radiation`, `precip_event_mm`,
  `rain_rate_mm` son campos extra dentro de forma WeatherAPI).

## Un solo repo, con la fuente seleccionable (decidido 2026-08-07)

**No se hace un repo aparte.** Las dos fuentes conviven en este mismo repo y se eligen
por configuración: WeatherAPI.com sigue siendo el valor por omisión y quien flashee el
binario público no nota ningún cambio.

El motivo es el tamaño del eje de variación: `DecodeWeatherAPI()` son ~200 de las 3 973
líneas del sketch, y el host es una constante. Duplicar el repo para cambiar el 5 % obliga
a arreglar cada bug dos veces. Ya hay prueba de en qué acaba eso: `WeatherAPI-Touch`
(3 391 líneas) y `WeatherAPI-Display-Touch` (3 973) nacieron iguales y hoy están a 580
líneas de distancia, sin que nadie sepa qué arreglos tiene cada uno.

Para terceros esto es una **función**, no lastre: apuntar el display a la estación propia
en vez de a WeatherAPI es atractivo para quien tiene un Ecowitt, y el servidor también es
público.

Coste asumido: el portal de configuración gana campos, el botón **Test API** debe
ramificar según la fuente, y los manuales ganan una sección **en tres idiomas**
(`MANUAL.md`, `MANUAL_EN.md`, `MANUAL_FR.md`).

---

## Fase 0 — Arreglar el amanecer (independiente, se puede hacer ya)

**Síntoma:** el amanecer y el atardecer se dibujan **una hora antes** de lo real.
Confirmado por el usuario.

**Causa:** `DecodeWeatherAPI()` líneas 1076‑1121 tiene un parche contra un bug de
WeatherAPI. Si el `tz_id` contiene `America/`, el aparato no tiene DST
(`daylightOffset_sec == 0`) y el mes está entre abril y octubre, asume que la API mandó
la astronomía adelantada una hora y le **resta 1 h** vía `adjustTimeByOffset()`
(línea 1671) al amanecer, atardecer y salida/puesta de luna.

El parche **envejeció**: se escribió cuando WeatherAPI aplicaba DST a México (abolido en
2022), y ahora que la fuente da la hora correcta el parche es el que introduce el error.
Con datos propios de pyephem el error sería permanente media parte del año.

Referencia de hoy (7 ago 2026, 19.380359 / ‑99.174564): amanecer real **06:14**,
atardecer **19:09** — pyephem y el cálculo solar independiente coinciden. La pantalla
dibuja 05:14.

**Arreglo:** eliminar la rama `apiMayHaveDST` y dejar siempre el cálculo de respaldo por
diferencia de horas (líneas 1106‑1121), que da `apiTimezoneOffset = 0` cuando los
relojes concuerdan y sigue sirviendo de red por si alguna vez hay un desajuste real de
zona horaria.

**Verificación:** el firmware ya imprime `Sun (raw)` y `Sun (adjusted)` en el monitor
serie (líneas 1198‑1199). Tras el arreglo deben ser idénticas.

---

## Fase 1 — Endpoint nuevo en el servidor ✅ HECHO Y DESPLEGADO (2026-08-07)

`GET https://clima.xe1e.net/api/epaper/forecast.json` está en producción y verificado con
dato real: `xe1e.source = "estacion"`, 3 días × 24 h, amanecer 06:14 y atardecer 19:09,
condiciones en español, fases lunares en inglés canónico, **22,8 KB** de payload frente al
buffer de 64 KB del firmware.

Implementado en `receiver/app/services/epaper.py` (commit `f8bee98` del repo del servidor),
con `openmeteo.get_forecast(..., epaper=True)` para el conjunto horario ampliado, cacheado
aparte del que usa el dashboard. Lo que sigue en esta sección es el diseño de referencia.

### Diseño

`GET /api/epaper/forecast.json` en `receiver/app/main.py`, con la lógica en un
`receiver/app/services/epaper.py` nuevo.

### `location` y `current`
Reusar `svitrix.build_weatherapi()`. Ya cubre temperatura, humedad, presión, viento
(velocidad/dirección/ráfaga/cardinal), UV, lluvia del día, condición + código, `is_day`
y calidad del aire.

Añadir lo que el e-paper consume y svitrix aún no emite:

| Campo | Origen |
|---|---|
| `feelslike_c` | estación (`/api/current`) |
| `dewpoint_c` | estación |
| `cloud` (%) | Open-Meteo `cloud_cover`, o derivado del índice de claridad que ya calcula `_condition()` |
| `vis_km` | METAR (`/api/metar`) u Open-Meteo — no se mide en la estación |
| `localtime`, `localtime_epoch`, `tz_id` | reloj del servidor (`America/Mexico_City`) |

### `forecast.forecastday[0..2]`
La estación no puede pronosticar; sale de Open-Meteo, ya cacheado en `/api/forecast`.

**Ampliar `receiver/app/services/openmeteo.py`** (misma llamada, sin coste extra):
- `_HOURLY` hoy solo pide `weather_code, temperature_2m, precipitation_probability`.
  Añadir `apparent_temperature`, `relative_humidity_2m`, `pressure_msl`,
  `wind_speed_10m`, `wind_direction_10m`, `precipitation`, `cloud_cover`, `is_day`.
- `_DAILY`: añadir `precipitation_sum` para `day.totalprecip_mm`.

Emitir `hour[]` con **las 24 horas** de cada día empezando a las 00:00 locales
(`timezone=auto` ya lo entrega así). El firmware muestrea cada 3 h (`h += 3`,
línea 1206) → 8 puntos/día × 3 días = 24 puntos, holgado frente a `max_readings 40`.

Campos por hora que el firmware lee: `time_epoch`, `time`, `temp_c`, `feelslike_c`,
`humidity`, `pressure_mb`, `wind_kph`, `wind_degree`, `precip_mm`, `chance_of_rain`,
`chance_of_snow`, `cloud`, `condition{code,text}`, `is_day`.

**Mapeo de códigos:** Open-Meteo entrega WMO; hay que traducir a código WeatherAPI
(el firmware ya hace WeatherAPI → icono OWM en `mapWeatherAPIIcon()`, línea 1252).
Tabla mecánica.

`day.maxtemp_c` / `mintemp_c` de **hoy**: usar los **medidos** de `/api/stats/daily`
combinados con el pronóstico para lo que resta del día — mejor que el dato de WeatherAPI.

### `astro`
De `get_almanac()` (pyephem, coordenadas exactas, sin el bug de DST).

Emitir las horas **directamente en `"HH:MM"` de 24 h**, que es lo que `_hhmm_local` ya
produce y lo que la pantalla dibuja. `convertTo24Hour()` deja pasar sin tocar las
cadenas sin `AM`/`PM`, así que no hay que cambiar el dibujado.

`moon_phase` debe ir en **inglés** para que `TranslateMoonPhase()` lo traduzca
(`almanac._moon_phase_name` devuelve español: hay que emitir el nombre canónico y dejar
la traducción al firmware, o mandar ambos). `moon_illumination` va como entero.

### Bloque `xe1e{}`
Datos exclusivos, para la Fase 3: radiación solar, lluvia del evento e intensidad,
IMECA real, sensores CH1‑8, remota GW1100, tendencia barométrica **real de 3 h** del
histórico, alertas activas, METAR/TAF, sismos.

### Política de disponibilidad
A diferencia de `/api/svitrix`, que responde 503 sin lecturas, este endpoint **nunca
debe fallar**: el e-paper se despierta, pide una vez y se vuelve a dormir, así que un
error deja la pantalla vacía hasta el siguiente ciclo. Sin dato de estación, caer al
`current` de Open-Meteo y marcarlo en `xe1e{}`.

### Ruta
Publicar en Caddy/nginx como el resto de `/api/*`.

---

## Fase 2 — Firmware: la fuente como opción

1. **Campos nuevos en `ConfigData`** (`wifi_manager.h:48‑74`), que hoy **no tiene ninguno
   de servidor**: `data_source` (0 = WeatherAPI.com, 1 = servidor propio) y `server_host`.
   Es lo que hace falta para que la variante sea configuración y no código — y de paso
   resuelve que el host sea hoy una constante de compilación
   (`owm_credentials.h:30`), imposible de reapuntar desde el portal.
2. **Ruta según la fuente** en `obtainWeatherData()`: `/v1/forecast.json` o
   `/api/epaper/forecast.json` (literal en la línea 1306). Los parámetros (`key`, `q`,
   `days`, `aqi`, `lang`) se pueden **dejar tal cual**: el endpoint propio los ignora,
   así que el constructor de la URI no necesita más cambios.
3. **Parser según la fuente.** `DecodeWeatherAPI()` queda **intacto** y al lado va
   `DecodeXE1E()`. Como el payload propio usa forma WeatherAPI de base, el segundo puede
   delegar en el primero y limitarse a leer el bloque `xe1e{}` y las diferencias.
4. **TLS.** Ya usa `client.setInsecure()` en puerto 443 (línea 1313) → el certificado de
   Caddy funciona sin tocar nada.
5. **Tendencia de presión.** Hoy se deduce del *pronóstico* (`WxForecast[0]` vs `[2]`,
   líneas 1236‑1243). Con fuente propia, usar la real de 3 h que manda el servidor.
6. **Botón "Test API"** del portal: hoy valida una clave de WeatherAPI y con la fuente
   propia reportaría fallo. Ramificar a un ping al endpoint propio.
7. **Portal y manuales:** campo nuevo en `CONFIG_PAGE` (`wifi_manager.h:77`) y una
   sección en los tres manuales.

**Sin tocar:** dibujado, iconos, touch, deep sleep, narrativa de Groq, calendario,
pantallas de radio, reloj mundial, `lang.h`, fuentes.

**Tamaño del JSON:** el buffer es `DynamicJsonDocument(64 * 1024)` y la respuesta de
WeatherAPI ronda 50 KB. La nuestra emite menos campos, así que cabe con holgura; aun
así, conviene medirla (`Response size` ya se imprime, línea 1320).

---

## Fase 3 — Lo que WeatherAPI nunca pudo dar (opcional, después)

Ya con el canal propio abierto:

- **Radiación solar** e **IMECA real** en vez del índice EPA estimado.
- **Lluvia del evento** e intensidad en mm/h.
- **Sensores CH1‑8** y la **remota GW1100**.
- **Pantalla de historia con histórico real de InfluxDB** en vez de que el aparato lo
  acumule en la SD (`weather_history.h`) — el servidor tiene años de dato medido.
- Alertas activas, METAR/TAF, sismos.

---

## Riesgos

| Riesgo | Mitigación |
|---|---|
| El e-paper se despierta y falla la petición → pantalla vacía un ciclo | El endpoint nunca devuelve error; cae a Open-Meteo |
| Mapeo WMO → WeatherAPI incompleto → icono equivocado | Tabla explícita + caso por omisión razonable |
| Nubosidad y visibilidad no se miden | Vienen de Open-Meteo/METAR, marcados en `xe1e{}` |
| Regresión silenciosa en el parseo | Comparar campo por campo la respuesta nueva contra una de WeatherAPI antes de flashear |

## Verificación

1. `curl` al endpoint nuevo y comparar campo por campo contra una respuesta real de
   WeatherAPI.
2. Monitor serie: `Response size`, `Sun (raw)` = `Sun (adjusted)`, y el conteo
   `Parsed N forecast periods` (debe dar 24).
3. Recorrer las 11+ pantallas en el aparato buscando `--` o valores absurdos.
