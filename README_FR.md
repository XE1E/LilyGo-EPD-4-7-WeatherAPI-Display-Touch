# LilyGo EPD 4.7" Weather Display - Version Touch

![Weather Station Display](LilyGo-EPD-4-7-WeatherAPI-Display-Touch.jpeg)

**[Espanol](README.md)** | **[English](README_EN.md)**

Station meteo pour ecran e-paper LilyGo T5 4.7" avec navigation tactile complete.

## Fonctionnalites

- **Navigation tactile** - 11+ ecrans navigables par touch
- **Meteo actuelle** - Temperature, humidite, pression, vent, Indice UV, Qualite de l'Air
- **Previsions 3 jours** - Predictions horaires et quotidiennes
- **Graphiques meteo** - Tendances temperature, pression, humidite, precipitation
- **Historique donnees** - Jusqu'a 1 an de donnees enregistrees (avec carte SD)
- **Phase lunaire** - Phase lunaire actuelle avec icone
- **Lever/coucher soleil** - Horaires solaires quotidiens
- **Calendrier** - Vues mensuelle et annuelle
- **Narrative meteo** - Descriptions generees par IA dans la langue selectionnee (Groq/Llama)
- **Citation du jour** - Citations inspirantes quotidiennes
- **Horloge mondiale** - Carte des fuseaux horaires
- **Configuration web** - Configure via WiFi AP, sans recompilation
- **Configuration Bluetooth** - App Android pour configuration BLE
- **Multi-langue** - Espagnol, Anglais, Francais
- **Multi-WiFi** - Se connecte au reseau le plus fort parmi 3 configures
- **Deep sleep** - Operation efficace en batterie avec intervalle configurable
- **Support carte SD** - Stockage etendu de l'historique (~52,000 lectures)

## Materiel

**Requis:** LilyGo T5 4.7" S3 Touch (ESP32-S3, e-paper 960x540, touch GT911)

Optionnel: Carte MicroSD pour historique etendu

## Demarrage Rapide

### 1. Televerser le Firmware

**Configuration Arduino IDE:**
| Parametre | Valeur |
|-----------|--------|
| Board | ESP32S3 Dev Module |
| USB CDC On Boot | Enable |
| USB DFU On Boot | Disable |
| Flash Size | 16MB (128Mb) |
| Flash Mode | QIO 80MHz |
| Partition Scheme | 16M Flash (3M APP/9.9MB FATFS) |
| PSRAM | OPI PSRAM |
| Upload Mode | UART0/Hardware CDC |
| USB Mode | Hardware CDC and JTAG |

**Bibliotheques Requises:**
- Board Manager: esp32 by Espressif Systems 2.0.17
- EPD47-master: https://github.com/DFRobotdl/EPD47/archive/refs/heads/master.zip
- ArduinoJson: by Benoit Blanchon 6.19.0

**Alternative: Installer via Web (sans Arduino IDE)**

[![Installer Firmware](https://img.shields.io/badge/Installer-Firmware-blue?style=for-the-badge)](https://xe1e.github.io/LilyGo-EPD-4-7-WeatherAPI-Display-Touch/)

Utiliser Chrome, Edge ou Opera et connecter l'appareil via USB.

### Mises a Jour du Firmware (OTA)

L'appareil supporte les mises a jour sans fil:

| Methode | URL / Comment utiliser |
|---------|------------------------|
| **Web OTA** | `http://[IP_APPAREIL]/ota` - Televerser .bin depuis navigateur |
| **Arduino OTA** | Selectionner le port reseau "WeatherStation" dans Arduino IDE |
| **Web Flasher** | [xe1e.github.io/LilyGo-EPD-4-7-WeatherAPI-Display-Touch](https://xe1e.github.io/LilyGo-EPD-4-7-WeatherAPI-Display-Touch/) |
| **Releases** | [Telecharger fichiers .bin](https://github.com/XE1E/LilyGo-EPD-4-7-WeatherAPI-Display-Touch/releases) |

### 2. Configuration Initiale

Au premier demarrage (ou sans WiFi disponible), l'appareil entre en mode configuration:

1. Se connecter au reseau WiFi: `WeatherStation-Setup`
2. Mot de passe: `weather123`
3. Ouvrir navigateur: `http://192.168.4.1`
4. Entrer vos parametres (cles API, emplacement, etc.)
5. Cliquer Sauvegarder - l'appareil redemarre et affiche la meteo

### 3. Fonctionnement Normal

Apres configuration, l'appareil:
1. Se connecte au WiFi
2. Recupere la meteo de WeatherAPI.com (un seul appel HTTPS)
3. Affiche la meteo a l'ecran
4. Permet 30 secondes de navigation tactile
5. Entre en deep sleep (intervalle configurable)
6. Se reveille et repete

## Navigation Tactile

| Zone de l'Ecran | Action |
|-----------------|--------|
| Grande icone meteo (centre) | Narrative Meteo (IA) |
| Zone batterie/WiFi (haut-droite) | Info Systeme |
| Zone lune/soleil (haut-gauche) | Calendrier |
| Temperature/Meteo (superieur) | Conditions Actuelles |
| Previsions horaires (milieu) | Previsions Etendues |
| Moitie gauche graphiques | Historique Meteo |
| Moitie droite graphiques | Tendances Previsions |

## Options de Configuration

| Champ | Description | Exemple |
|-------|-------------|---------|
| WiFi (jusqu'a 3) | SSID et mot de passe | MonWiFi / motdepasse123 |
| WeatherAPI Key | Cle API WeatherAPI.com | abc123... |
| Groq API Key | Pour narrative IA (gratuit) | gsk_... |
| Ville | Nom de ville a afficher | Paris |
| Latitude | Latitude de l'emplacement | 48.8566 |
| Longitude | Longitude de l'emplacement | 2.3522 |
| Fuseau Horaire | Chaine POSIX de fuseau horaire | CET-1CEST |
| Intervalle | Minutes entre mises a jour | 30 |
| Langue | Langue de l'interface | ES / EN / FR |
| Unites | Metrique ou Imperial | M / I |
| Style narrative | Style du texte IA | Radio, Formel, Poetique... |

## Cles API

### WeatherAPI.com (requis)
1. S'inscrire sur https://www.weatherapi.com/
2. Aller au Dashboard
3. Copier votre cle API

### Groq (optionnel, pour Narrative Meteo)
1. S'inscrire sur https://console.groq.com/
2. Creer une cle API
3. Tier gratuit: 30 requetes/minute

## Ecrans

| # | Ecran | Acces |
|---|-------|-------|
| 1 | Meteo Principale | Par defaut |
| 2 | Conditions Actuelles | Toucher zone superieure |
| 3 | Previsions Etendues | Toucher icones horaires |
| 4 | Tendances Meteo | Toucher graphiques droite |
| 5 | Historique Meteo | Toucher graphiques gauche |
| 6 | Info Systeme (5 pages) | Toucher batterie/WiFi |
| 7 | Qualite de l'Air | Depuis Conditions Actuelles |
| 8 | Calendrier (Mensuel/Annuel) | Toucher zone lune/soleil |
| 9 | Narrative Meteo | Toucher grande icone |
| 10 | Citation du Jour | Cache: Info > Credits > ... |
| 11 | Horloge Mondiale | Navigation cachee |

## Depannage

### Echec du televersement
1. Appuyer et maintenir le bouton BOOT
2. En maintenant BOOT, appuyer sur RST
3. Relacher RST, puis relacher BOOT
4. Le televersement devrait fonctionner

### Touch ne repond pas
- Nettoyer la surface de l'ecran
- Verifier la detection GT911 dans le Moniteur Serie
- Toucher fermement pendant au moins 50ms

### Pas de donnees meteo
- Verifier que votre cle API WeatherAPI.com est valide
- Erreur 403 = cle API invalide
- Verifier latitude/longitude correctes
- Verifier identifiants WiFi

### Fantomes sur l'ecran
- Utiliser le bouton "Nettoyer Ecran" dans Info Systeme
- Ou redemarrer l'appareil

## Fichiers

| Fichier | Description |
|---------|-------------|
| `LilyGo-EPD-4-7-WeatherAPI-Display-Touch.ino` | Sketch principal |
| `wifi_manager.h` | Mode AP et serveur web |
| `owm_credentials.h` | Configuration par defaut |
| `lang.h` | Chaines multi-langue |
| `touch_handler.h` | Navigation tactile |
| `weather_narrative.h` | Descriptions IA meteo |
| `weather_history.h` | Systeme de stockage |
| `calendar.h` | Vues calendrier |
| `quote_screen.h` | Citation du jour |
| `forecast_record.h` | Structure de donnees |
| `opensans*.h` | Fichiers de polices |

## Details Techniques

- **Ecran:** 960x540 pixels, niveaux de gris 4-bit
- **Touch:** Controleur capacitif GT911
- **MCU:** ESP32-S3 avec 8MB PSRAM
- **Consommation:** Deep sleep entre mises a jour (~10uA)
- **Stockage:** NVS + FFat (~7 jours) + carte SD (~1 an)
- **Source de reveil:** Timer

## Documentation

Voir les manuels detailles en trois langues:
- [MANUAL.md](MANUAL.md) - Espanol
- [MANUAL_EN.md](MANUAL_EN.md) - English
- [MANUAL_FR.md](MANUAL_FR.md) - Francais

## Licence

Base sur le travail original de David Bird - Voir Licence.txt

## Credits

- Code original par David Bird 2021
- Port ESP32 par Xinyuan-LilyGO
- Modifie par Stefan Maetschke 2025
- Version touch avec extensions par XE1E 2026
- Bibliotheque EPD47 par Vroland/DFRobot
- Donnees meteo de WeatherAPI.com
- Narrative IA par Groq/Llama

---

73 de XE1E
