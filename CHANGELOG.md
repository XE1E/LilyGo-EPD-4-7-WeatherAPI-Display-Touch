# Changelog

Todos los cambios notables de este proyecto se documentan en este archivo.

El formato está basado en [Keep a Changelog](https://keepachangelog.com/es-ES/1.1.0/)
y el proyecto sigue un versionado de tipo `MAYOR.MENOR`.

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
