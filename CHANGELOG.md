# Changelog

Todos los cambios notables de este proyecto se documentan en este archivo.

El formato está basado en [Keep a Changelog](https://keepachangelog.com/es-ES/1.1.0/)
y el proyecto sigue un versionado de tipo `MAYOR.MENOR`.

## [No publicado]

### Corregido
- **Amanecer y atardecer se dibujaban una hora antes de lo real** entre abril y octubre. Había un heurístico que, para cualquier zona horaria `America/*` sin DST en el aparato y dentro de esos meses, daba por hecho que la fuente mandaba la astronomía adelantada una hora y se la restaba. Se escribió cuando WeatherAPI aplicaba DST a México (abolido en 2022); ahora que la fuente devuelve la hora correcta, el parche era el que introducía el error. Afectaba también a la salida y puesta de la luna.
- El desfase de zona horaria entre la fuente y el aparato ahora se compara en minutos y solo se redondea a hora completa a partir de los 30 minutos. Comparando horas enteras, una consulta que cruzara el cambio de hora (la fuente sella `06:59` cuando el aparato ya marca `07:00`) producía un desfase falso de una hora.

## [2.9] - 2026-06-26

### Añadido
- Validaciones en el flujo de configuración web: **Test WiFi** (verifica que el SSID existe y muestra la intensidad de señal) y **Test API** (valida la clave de WeatherAPI con una llamada real).
- Guía de instalación y mejoras de visibilidad del **Web Flasher**.

### Cambiado
- Sketch renombrado para coincidir con el nombre de la carpeta (compatibilidad con Arduino IDE).
- Pantalla del modo AP (**Configuración Inicial**): título y subtítulo subidos 15px para un mejor espaciado vertical.
- Mejoras generales en el flujo de configuración y en la documentación.

### Seguridad
- Eliminadas las claves API que estaban expuestas en el archivo de credenciales.
- Historial de git purgado (BFG) para eliminar los secretos de commits anteriores; claves rotadas.

### Notas
- Se intentó traducir los mensajes de la herramienta esp-web-tools del Web Flasher; el cambio se revirtió, por lo que esos mensajes permanecen en inglés.

## [2.8] - 2026-05-26

- Primera versión etiquetada y publicada con compilación automática de firmware (CI) y Web Flasher.
- Estación meteorológica para LilyGo EPD 4.7" con navegación táctil, integración con WeatherAPI.com, soporte multi-WiFi, portal cautivo en modo AP, historial de datos y deep sleep para operación con batería.

[2.9]: https://github.com/XE1E/LilyGo-EPD-4-7-WeatherAPI-Display-Touch/releases/tag/v2.9
[2.8]: https://github.com/XE1E/LilyGo-EPD-4-7-WeatherAPI-Display-Touch/releases/tag/v2.8
