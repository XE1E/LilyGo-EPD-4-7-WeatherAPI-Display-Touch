#ifndef LANG_H
#define LANG_H

#define FONT(x) x##_tf

// Language codes
#define LANG_ES 0  // Spanish
#define LANG_EN 1  // English
#define LANG_FR 2  // French

// Current language (default Spanish)
int currentLang = LANG_ES;

// Set language based on config string
void setLanguage(const char* lang) {
  if (strcmp(lang, "EN") == 0 || strcmp(lang, "en") == 0) {
    currentLang = LANG_EN;
  } else if (strcmp(lang, "FR") == 0 || strcmp(lang, "fr") == 0) {
    currentLang = LANG_FR;
  } else {
    currentLang = LANG_ES;  // Default to Spanish
  }
}

// ============== TEMPERATURE / HUMIDITY ==============
const char* TXT_TEMPERATURE_C_ARR[] = { "Temperatura (°C)", "Temperature (°C)", "Temperature (°C)" };
const char* TXT_TEMPERATURE_F_ARR[] = { "Temperatura (°F)", "Temperature (°F)", "Temperature (°F)" };
const char* TXT_HUMIDITY_PERCENT_ARR[] = { "Humedad (%)", "Humidity (%)", "Humidite (%)" };

#define TXT_TEMPERATURE_C String(TXT_TEMPERATURE_C_ARR[currentLang])
#define TXT_TEMPERATURE_F String(TXT_TEMPERATURE_F_ARR[currentLang])
#define TXT_HUMIDITY_PERCENT String(TXT_HUMIDITY_PERCENT_ARR[currentLang])

// ============== PRESSURE ==============
const char* TXT_PRESSURE_HPA_ARR[] = { "Presion (mb)", "Pressure (hPa)", "Pression (hPa)" };
const char* TXT_PRESSURE_IN_ARR[] = { "Presion (in)", "Pressure (in)", "Pression (in)" };
const char* TXT_PRESSURE_STEADY_ARR[] = { "Estable", "Steady", "Stable" };
const char* TXT_PRESSURE_RISING_ARR[] = { "Subiendo", "Rising", "En hausse" };
const char* TXT_PRESSURE_FALLING_ARR[] = { "Bajando", "Falling", "En baisse" };

#define TXT_PRESSURE_HPA String(TXT_PRESSURE_HPA_ARR[currentLang])
#define TXT_PRESSURE_IN String(TXT_PRESSURE_IN_ARR[currentLang])
#define TXT_PRESSURE_STEADY String(TXT_PRESSURE_STEADY_ARR[currentLang])
#define TXT_PRESSURE_RISING String(TXT_PRESSURE_RISING_ARR[currentLang])
#define TXT_PRESSURE_FALLING String(TXT_PRESSURE_FALLING_ARR[currentLang])

// ============== RAINFALL / SNOWFALL ==============
const char* TXT_RAINFALL_MM_ARR[] = { "Lluvia (mm)", "Rainfall (mm)", "Pluie (mm)" };
const char* TXT_RAINFALL_IN_ARR[] = { "Lluvia (in)", "Rainfall (in)", "Pluie (in)" };
const char* TXT_SNOWFALL_MM_ARR[] = { "Nevada (mm)", "Snowfall (mm)", "Neige (mm)" };
const char* TXT_SNOWFALL_IN_ARR[] = { "Nevada (in)", "Snowfall (in)", "Neige (in)" };
const char* TXT_PRECIPITATION_SOON_ARR[] = { "Prec.", "Prec.", "Prec." };

#define TXT_RAINFALL_MM String(TXT_RAINFALL_MM_ARR[currentLang])
#define TXT_RAINFALL_IN String(TXT_RAINFALL_IN_ARR[currentLang])
#define TXT_SNOWFALL_MM String(TXT_SNOWFALL_MM_ARR[currentLang])
#define TXT_SNOWFALL_IN String(TXT_SNOWFALL_IN_ARR[currentLang])
#define TXT_PRECIPITATION_SOON String(TXT_PRECIPITATION_SOON_ARR[currentLang])

// ============== SUN ==============
const char* TXT_SUNRISE_ARR[] = { "Amanecer", "Sunrise", "Lever" };
const char* TXT_SUNSET_ARR[] = { "Anochecer", "Sunset", "Coucher" };

#define TXT_SUNRISE String(TXT_SUNRISE_ARR[currentLang])
#define TXT_SUNSET String(TXT_SUNSET_ARR[currentLang])

// ============== MOON PHASES ==============
const char* TXT_MOON_NEW_ARR[] = { "Luna Nueva", "New Moon", "Nouvelle Lune" };
const char* TXT_MOON_WAXING_CRESCENT_ARR[] = { "Luna Creciente", "Waxing Crescent", "Premier Croissant" };
const char* TXT_MOON_FIRST_QUARTER_ARR[] = { "Cuarto Creciente", "First Quarter", "Premier Quartier" };
const char* TXT_MOON_WAXING_GIBBOUS_ARR[] = { "Gibosa Creciente", "Waxing Gibbous", "Gibbeuse Croissante" };
const char* TXT_MOON_FULL_ARR[] = { "Luna Llena", "Full Moon", "Pleine Lune" };
const char* TXT_MOON_WANING_GIBBOUS_ARR[] = { "Gibosa Menguante", "Waning Gibbous", "Gibbeuse Decroissante" };
const char* TXT_MOON_THIRD_QUARTER_ARR[] = { "Cuarto Menguante", "Third Quarter", "Dernier Quartier" };
const char* TXT_MOON_WANING_CRESCENT_ARR[] = { "Luna Menguante", "Waning Crescent", "Dernier Croissant" };

#define TXT_MOON_NEW String(TXT_MOON_NEW_ARR[currentLang])
#define TXT_MOON_WAXING_CRESCENT String(TXT_MOON_WAXING_CRESCENT_ARR[currentLang])
#define TXT_MOON_FIRST_QUARTER String(TXT_MOON_FIRST_QUARTER_ARR[currentLang])
#define TXT_MOON_WAXING_GIBBOUS String(TXT_MOON_WAXING_GIBBOUS_ARR[currentLang])
#define TXT_MOON_FULL String(TXT_MOON_FULL_ARR[currentLang])
#define TXT_MOON_WANING_GIBBOUS String(TXT_MOON_WANING_GIBBOUS_ARR[currentLang])
#define TXT_MOON_THIRD_QUARTER String(TXT_MOON_THIRD_QUARTER_ARR[currentLang])
#define TXT_MOON_WANING_CRESCENT String(TXT_MOON_WANING_CRESCENT_ARR[currentLang])

// ============== MISC ==============
const char* TXT_POWER_ARR[] = { "Energia", "Power", "Energie" };
const char* TXT_WIFI_ARR[] = { "WiFi", "WiFi", "WiFi" };
const char* TXT_UPDATED_ARR[] = { "Actualizado:", "Updated:", "Mis a jour:" };

#define TXT_POWER String(TXT_POWER_ARR[currentLang])
#define TXT_WIFI String(TXT_WIFI_ARR[currentLang])
const char* TXT_UPDATED = "Updated:";  // Serial output always in English

// ============== AP MODE / SLEEP ==============
const char* TXT_AP_WIFI_CONFIG_MODE_ARR[] = { "Modo Configuracion WiFi", "WiFi Configuration Mode", "Mode Configuration WiFi" };
const char* TXT_AP_CONNECT_TO_WIFI_ARR[] = { "Conectar a la red WiFi:", "Connect to WiFi network:", "Connectez-vous au reseau WiFi:" };
const char* TXT_AP_PASSWORD_ARR[] = { "Contrasena:", "Password:", "Mot de passe:" };
const char* TXT_AP_OPEN_BROWSER_ARR[] = { "Luego abrir en navegador:", "Then open in browser:", "Puis ouvrir dans le navigateur:" };
const char* TXT_AP_DEVICE_RESTART_ARR[] = { "El dispositivo se reiniciara al guardar", "Device will restart when saving", "L'appareil redemarrera a l'enregistrement" };
const char* TXT_AP_ATTEMPTS_REMAINING_ARR[] = { "Intentos restantes:", "Attempts remaining:", "Tentatives restantes:" };
const char* TXT_AP_WIFI_FAILED_ARR[] = { "No se pudo conectar a WiFi", "Could not connect to WiFi", "Impossible de se connecter au WiFi" };
const char* TXT_AP_ATTEMPTS_EXHAUSTED_ARR[] = { "Intentos agotados", "Attempts exhausted", "Tentatives epuisees" };
const char* TXT_AP_PERMANENT_SLEEP_ARR[] = { "Entrando en modo sleep permanente", "Entering permanent sleep mode", "Entree en mode veille permanent" };
const char* TXT_AP_PRESS_RESET_ARR[] = { "Presione RESET para reiniciar", "Press RESET to restart", "Appuyez sur RESET pour redemarrer" };

#define TXT_AP_WIFI_CONFIG_MODE String(TXT_AP_WIFI_CONFIG_MODE_ARR[currentLang])
#define TXT_AP_CONNECT_TO_WIFI String(TXT_AP_CONNECT_TO_WIFI_ARR[currentLang])
#define TXT_AP_PASSWORD String(TXT_AP_PASSWORD_ARR[currentLang])
#define TXT_AP_OPEN_BROWSER String(TXT_AP_OPEN_BROWSER_ARR[currentLang])
#define TXT_AP_DEVICE_RESTART String(TXT_AP_DEVICE_RESTART_ARR[currentLang])
#define TXT_AP_ATTEMPTS_REMAINING String(TXT_AP_ATTEMPTS_REMAINING_ARR[currentLang])
#define TXT_AP_WIFI_FAILED String(TXT_AP_WIFI_FAILED_ARR[currentLang])
#define TXT_AP_ATTEMPTS_EXHAUSTED String(TXT_AP_ATTEMPTS_EXHAUSTED_ARR[currentLang])
#define TXT_AP_PERMANENT_SLEEP String(TXT_AP_PERMANENT_SLEEP_ARR[currentLang])
#define TXT_AP_PRESS_RESET String(TXT_AP_PRESS_RESET_ARR[currentLang])

// ============== UI LABELS ==============
const char* TXT_VERSION_ARR[] = { "Version:", "Version:", "Version:" };
const char* TXT_COMPILED_ARR[] = { "Compilado:", "Compiled:", "Compile:" };
const char* TXT_FREE_HEAP_ARR[] = { "Heap Libre:", "Free Heap:", "Heap Libre:" };
const char* TXT_TOTAL_HEAP_ARR[] = { "Heap Total:", "Total Heap:", "Heap Total:" };
const char* TXT_DISCONNECTED_ARR[] = { "Desconectado", "Disconnected", "Deconnecte" };
const char* TXT_WIFI_SIGNAL_ARR[] = { "Senal WiFi:", "WiFi Signal:", "Signal WiFi:" };
const char* TXT_CITY_ARR[] = { "Ciudad:", "City:", "Ville:" };
const char* TXT_COORDINATES_ARR[] = { "Coordenadas:", "Coordinates:", "Coordonnees:" };
const char* TXT_TIMEZONE_ARR[] = { "Zona Horaria:", "Timezone:", "Fuseau Horaire:" };
const char* TXT_LANGUAGE_ARR[] = { "Idioma:", "Language:", "Langue:" };
const char* TXT_UNITS_ARR[] = { "Unidades:", "Units:", "Unites:" };
const char* TXT_METRIC_ARR[] = { "Metrico", "Metric", "Metrique" };
const char* TXT_IMPERIAL_ARR[] = { "Imperial", "Imperial", "Imperial" };
const char* TXT_HEMISPHERE_ARR[] = { "Hemisferio:", "Hemisphere:", "Hemisphere:" };
const char* TXT_NORTH_ARR[] = { "Norte", "North", "Nord" };
const char* TXT_SOUTH_ARR[] = { "Sur", "South", "Sud" };
const char* TXT_FORECAST_ARR[] = { "Pronostico:", "Forecast:", "Previsions:" };
const char* TXT_DAYS_ARR[] = { "dias", "days", "jours" };
const char* TXT_UPDATE_INTERVAL_ARR[] = { "Actualizacion:", "Update:", "Mise a jour:" };
const char* TXT_ON_SLEEP_ARR[] = { "Al Dormir:", "On Sleep:", "En Veille:" };
const char* TXT_KEEP_SCREEN_ARR[] = { "Mantener pantalla", "Keep screen", "Garder ecran" };
const char* TXT_GO_MAIN_ARR[] = { "Ir a principal", "Go to main", "Aller au principal" };
const char* TXT_WEB_CONFIG_ARR[] = { "Config Web:", "Web Config:", "Config Web:" };
const char* TXT_WIFI_DISCONNECTED_ARR[] = { "WiFi desconectado", "WiFi disconnected", "WiFi deconnecte" };
const char* TXT_CLEAN_SCREEN_ARR[] = { "Limpiar Pantalla", "Clean Screen", "Nettoyer Ecran" };
const char* TXT_UPTIME_ARR[] = { "Tiempo activo:", "Uptime:", "Temps actif:" };
const char* TXT_HISTORY_ARR[] = { "Historial", "History", "Historique" };
const char* TXT_NOT_ENOUGH_DATA_ARR[] = { "Aun no hay suficientes datos", "Not enough data yet", "Pas encore assez de donnees" };
const char* TXT_HISTORY_FILLS_ARR[] = { "El historial se llena con cada actualizacion", "History fills with each update", "L'historique se remplit a chaque maj" };
const char* TXT_INSUFFICIENT_DATA_ARR[] = { "Datos insuficientes para graficar", "Insufficient data for graph", "Donnees insuffisantes pour graphique" };
const char* TXT_READINGS_ARR[] = { "lecturas", "readings", "lectures" };
const char* TXT_NEXT_DAYS_ARR[] = { "Proximos 3 dias", "Next 3 days", "Prochains 3 jours" };
const char* TXT_GRAPH_TEMP_ARR[] = { "Temperatura", "Temperature", "Temperature" };
const char* TXT_GRAPH_PRESSURE_ARR[] = { "Presion", "Pressure", "Pression" };
const char* TXT_GRAPH_HUMIDITY_ARR[] = { "Humedad", "Humidity", "Humidite" };
const char* TXT_GRAPH_RAIN_ARR[] = { "Lluvia", "Rainfall", "Pluie" };
const char* TXT_MAX_ARR[] = { "Max", "Max", "Max" };
const char* TXT_MIN_ARR[] = { "Min", "Min", "Min" };

#define TXT_VERSION String(TXT_VERSION_ARR[currentLang])
#define TXT_COMPILED String(TXT_COMPILED_ARR[currentLang])
#define TXT_FREE_HEAP String(TXT_FREE_HEAP_ARR[currentLang])
#define TXT_TOTAL_HEAP String(TXT_TOTAL_HEAP_ARR[currentLang])
#define TXT_DISCONNECTED String(TXT_DISCONNECTED_ARR[currentLang])
#define TXT_WIFI_SIGNAL String(TXT_WIFI_SIGNAL_ARR[currentLang])
#define TXT_CITY String(TXT_CITY_ARR[currentLang])
#define TXT_COORDINATES String(TXT_COORDINATES_ARR[currentLang])
#define TXT_TIMEZONE String(TXT_TIMEZONE_ARR[currentLang])
#define TXT_LANGUAGE String(TXT_LANGUAGE_ARR[currentLang])
#define TXT_UNITS String(TXT_UNITS_ARR[currentLang])
#define TXT_METRIC String(TXT_METRIC_ARR[currentLang])
#define TXT_IMPERIAL String(TXT_IMPERIAL_ARR[currentLang])
#define TXT_HEMISPHERE String(TXT_HEMISPHERE_ARR[currentLang])
#define TXT_NORTH String(TXT_NORTH_ARR[currentLang])
#define TXT_SOUTH String(TXT_SOUTH_ARR[currentLang])
#define TXT_FORECAST String(TXT_FORECAST_ARR[currentLang])
#define TXT_DAYS String(TXT_DAYS_ARR[currentLang])
#define TXT_UPDATE_INTERVAL String(TXT_UPDATE_INTERVAL_ARR[currentLang])
#define TXT_ON_SLEEP String(TXT_ON_SLEEP_ARR[currentLang])
#define TXT_KEEP_SCREEN String(TXT_KEEP_SCREEN_ARR[currentLang])
#define TXT_GO_MAIN String(TXT_GO_MAIN_ARR[currentLang])
#define TXT_WEB_CONFIG String(TXT_WEB_CONFIG_ARR[currentLang])
#define TXT_WIFI_DISCONNECTED String(TXT_WIFI_DISCONNECTED_ARR[currentLang])
#define TXT_CLEAN_SCREEN String(TXT_CLEAN_SCREEN_ARR[currentLang])
#define TXT_UPTIME String(TXT_UPTIME_ARR[currentLang])
#define TXT_HISTORY String(TXT_HISTORY_ARR[currentLang])
#define TXT_NOT_ENOUGH_DATA String(TXT_NOT_ENOUGH_DATA_ARR[currentLang])
#define TXT_HISTORY_FILLS String(TXT_HISTORY_FILLS_ARR[currentLang])
#define TXT_INSUFFICIENT_DATA String(TXT_INSUFFICIENT_DATA_ARR[currentLang])
#define TXT_READINGS String(TXT_READINGS_ARR[currentLang])
#define TXT_NEXT_DAYS String(TXT_NEXT_DAYS_ARR[currentLang])
#define TXT_GRAPH_TEMP String(TXT_GRAPH_TEMP_ARR[currentLang])
#define TXT_GRAPH_PRESSURE String(TXT_GRAPH_PRESSURE_ARR[currentLang])
#define TXT_GRAPH_HUMIDITY String(TXT_GRAPH_HUMIDITY_ARR[currentLang])
#define TXT_GRAPH_RAIN String(TXT_GRAPH_RAIN_ARR[currentLang])
#define TXT_MAX String(TXT_MAX_ARR[currentLang])
#define TXT_MIN String(TXT_MIN_ARR[currentLang])

const char* TXT_48H_ARR[] = { "48 H", "48 H", "48 H" };
const char* TXT_1WEEK_ARR[] = { "1 SEM", "1 WEEK", "1 SEM" };
const char* TXT_SEC_ARR[] = { "seg", "sec", "sec" };

#define TXT_48H String(TXT_48H_ARR[currentLang])
#define TXT_1WEEK String(TXT_1WEEK_ARR[currentLang])
#define TXT_SEC String(TXT_SEC_ARR[currentLang])

// ============== WIND DIRECTIONS ==============
// Spanish: O=Oeste, English: W=West, French: O=Ouest
const char* TXT_N_ARR[]   = { "N",   "N",   "N" };
const char* TXT_NNE_ARR[] = { "NNE", "NNE", "NNE" };
const char* TXT_NE_ARR[]  = { "NE",  "NE",  "NE" };
const char* TXT_ENE_ARR[] = { "ENE", "ENE", "ENE" };
const char* TXT_E_ARR[]   = { "E",   "E",   "E" };
const char* TXT_ESE_ARR[] = { "ESE", "ESE", "ESE" };
const char* TXT_SE_ARR[]  = { "SE",  "SE",  "SE" };
const char* TXT_SSE_ARR[] = { "SSE", "SSE", "SSE" };
const char* TXT_S_ARR[]   = { "S",   "S",   "S" };
const char* TXT_SSW_ARR[] = { "SSO", "SSW", "SSO" };
const char* TXT_SW_ARR[]  = { "SO",  "SW",  "SO" };
const char* TXT_WSW_ARR[] = { "OSO", "WSW", "OSO" };
const char* TXT_W_ARR[]   = { "O",   "W",   "O" };
const char* TXT_WNW_ARR[] = { "ONO", "WNW", "ONO" };
const char* TXT_NW_ARR[]  = { "NO",  "NW",  "NO" };
const char* TXT_NNW_ARR[] = { "NNO", "NNW", "NNO" };

#define TXT_N   String(TXT_N_ARR[currentLang])
#define TXT_NNE String(TXT_NNE_ARR[currentLang])
#define TXT_NE  String(TXT_NE_ARR[currentLang])
#define TXT_ENE String(TXT_ENE_ARR[currentLang])
#define TXT_E   String(TXT_E_ARR[currentLang])
#define TXT_ESE String(TXT_ESE_ARR[currentLang])
#define TXT_SE  String(TXT_SE_ARR[currentLang])
#define TXT_SSE String(TXT_SSE_ARR[currentLang])
#define TXT_S   String(TXT_S_ARR[currentLang])
#define TXT_SSW String(TXT_SSW_ARR[currentLang])
#define TXT_SW  String(TXT_SW_ARR[currentLang])
#define TXT_WSW String(TXT_WSW_ARR[currentLang])
#define TXT_W   String(TXT_W_ARR[currentLang])
#define TXT_WNW String(TXT_WNW_ARR[currentLang])
#define TXT_NW  String(TXT_NW_ARR[currentLang])
#define TXT_NNW String(TXT_NNW_ARR[currentLang])

// ============== DAYS OF WEEK ==============
const char* weekday_ES[] = { "Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado" };
const char* weekday_EN[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
const char* weekday_FR[] = { "Dim", "Lun", "Mar", "Mer", "Jeu", "Ven", "Sam" };

// Short 3-letter abbreviations for calendar (Monday first)
const char* weekday_short_ES[] = { "LUN", "MAR", "MIE", "JUE", "VIE", "SAB", "DOM" };
const char* weekday_short_EN[] = { "MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN" };
const char* weekday_short_FR[] = { "LUN", "MAR", "MER", "JEU", "VEN", "SAM", "DIM" };

// Mini 1-letter abbreviations for yearly calendar (Monday first)
const char* weekday_mini_ES[] = { "L", "M", "M", "J", "V", "S", "D" };
const char* weekday_mini_EN[] = { "M", "T", "W", "T", "F", "S", "S" };
const char* weekday_mini_FR[] = { "L", "M", "M", "J", "V", "S", "D" };

const char* getWeekdayShort(int day) {
  switch (currentLang) {
    case LANG_EN: return weekday_short_EN[day];
    case LANG_FR: return weekday_short_FR[day];
    default: return weekday_short_ES[day];
  }
}

const char* getWeekdayMini(int day) {
  switch (currentLang) {
    case LANG_EN: return weekday_mini_EN[day];
    case LANG_FR: return weekday_mini_FR[day];
    default: return weekday_mini_ES[day];
  }
}

const char* getWeekday(int day) {
  switch (currentLang) {
    case LANG_EN: return weekday_EN[day];
    case LANG_FR: return weekday_FR[day];
    default: return weekday_ES[day];
  }
}

// For compatibility - pointer to current weekday array
const char** weekday_D = weekday_ES;

void updateWeekdayPointer() {
  switch (currentLang) {
    case LANG_EN: weekday_D = weekday_EN; break;
    case LANG_FR: weekday_D = weekday_FR; break;
    default: weekday_D = weekday_ES; break;
  }
}

// ============== MONTHS ==============
const char* month_ES[] = { "Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio", "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre" };
const char* month_EN[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
const char* month_FR[] = { "Jan", "Fev", "Mar", "Avr", "Mai", "Juin", "Juil", "Aout", "Sep", "Oct", "Nov", "Dec" };

// Full month names for calendar
const char* month_full_ES[] = { "Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio", "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre" };
const char* month_full_EN[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
const char* month_full_FR[] = { "Janvier", "Fevrier", "Mars", "Avril", "Mai", "Juin", "Juillet", "Aout", "Septembre", "Octobre", "Novembre", "Decembre" };

// Uppercase month names for yearly calendar
const char* month_upper_ES[] = { "ENERO", "FEBRERO", "MARZO", "ABRIL", "MAYO", "JUNIO", "JULIO", "AGOSTO", "SEPTIEMBRE", "OCTUBRE", "NOVIEMBRE", "DICIEMBRE" };
const char* month_upper_EN[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
const char* month_upper_FR[] = { "JAN", "FEV", "MAR", "AVR", "MAI", "JUIN", "JUIL", "AOUT", "SEP", "OCT", "NOV", "DEC" };

const char* getMonth(int month) {
  switch (currentLang) {
    case LANG_EN: return month_EN[month];
    case LANG_FR: return month_FR[month];
    default: return month_ES[month];
  }
}

const char* getMonthFull(int month) {
  switch (currentLang) {
    case LANG_EN: return month_full_EN[month];
    case LANG_FR: return month_full_FR[month];
    default: return month_full_ES[month];
  }
}

const char* getMonthUpper(int month) {
  switch (currentLang) {
    case LANG_EN: return month_upper_EN[month];
    case LANG_FR: return month_upper_FR[month];
    default: return month_upper_ES[month];
  }
}

// For compatibility
const char** month_M = month_ES;

void updateMonthPointer() {
  switch (currentLang) {
    case LANG_EN: month_M = month_EN; break;
    case LANG_FR: month_M = month_FR; break;
    default: month_M = month_ES; break;
  }
}

// ============== NAVIGATION SCREENS ==============
const char* TXT_BACK_ARR[] = { "Regresar", "Back", "Retour" };
const char* TXT_CURRENT_CONDITIONS_ARR[] = { "Condiciones Actuales", "Current Conditions", "Conditions Actuelles" };
const char* TXT_5DAY_FORECAST_ARR[] = { "Pronostico", "Forecast", "Previsions" };
const char* TXT_WEATHER_TRENDS_ARR[] = { "Tendencia del Clima", "Weather Trends", "Tendances Meteo" };
const char* TXT_NEXT_24H_ARR[] = { "Proximas 24 Horas", "Next 24 Hours", "Prochaines 24 Heures" };
const char* TXT_5DAY_OUTLOOK_ARR[] = { "Proximos 3 Dias", "Next 3 Days", "Prochains 3 Jours" };
const char* TXT_SYSTEM_INFO_ARR[] = { "CONFIGURACION DEL SISTEMA", "SYSTEM CONFIGURATION", "CONFIGURATION SYSTEME" };
const char* TXT_INFO_HARDWARE_ARR[] = { "CARACTERISTICAS - HARDWARE", "FEATURES - HARDWARE", "CARACTERISTIQUES - HARDWARE" };
const char* TXT_INFO_SOFTWARE_ARR[] = { "CARACTERISTICAS - SOFTWARE", "FEATURES - SOFTWARE", "CARACTERISTIQUES - SOFTWARE" };
const char* TXT_INFO_HELP_ARR[] = { "AYUDA RAPIDA", "QUICK HELP", "AIDE RAPIDE" };
const char* TXT_INFO_CREDITS_ARR[] = { "CREDITOS", "CREDITS", "CREDITS" };

// Help screen texts
const char* TXT_HELP_NAV_ARR[] = { "Navegacion - (touch)", "Navigation - (touch)", "Navigation - (touch)" };
const char* TXT_HELP_CALENDAR_ARR[] = { "* Luna/Sol: Calendario (touch largo)", "* Moon/Sun: Calendar (long touch)", "* Lune/Soleil: Calendrier (touch long)" };
const char* TXT_HELP_ZONE_ARR[] = { "* Zona superior central: Condiciones actuales", "* Upper central zone: Current conditions", "* Zone centrale superieure: Conditions actuelles" };
const char* TXT_HELP_ICONS_ARR[] = { "* Iconos pronostico: Pronostico extendido", "* Forecast icons: Extended forecast", "* Icones previsions: Previsions etendues" };
const char* TXT_HELP_GRAPH_LEFT_ARR[] = { "* Graficas izquierda: Historial real", "* Left graphs: Real history", "* Graphiques gauche: Historique reel" };
const char* TXT_HELP_GRAPH_RIGHT_ARR[] = { "* Graficas derecha: Tendencias futuras", "* Right graphs: Future trends", "* Graphiques droite: Tendances futures" };
const char* TXT_HELP_BATTERY_ARR[] = { "* Bateria/WiFi: Esta pantalla", "* Battery/WiFi: This screen", "* Batterie/WiFi: Cet ecran" };
const char* TXT_HELP_INITIAL_ARR[] = { "Configuracion Inicial", "Initial Setup", "Configuration Initiale" };
const char* TXT_HELP_STEP1_ARR[] = { "1. Conectar a WiFi: WeatherStation-Setup", "1. Connect to WiFi: WeatherStation-Setup", "1. Connecter au WiFi: WeatherStation-Setup" };
const char* TXT_HELP_STEP2_ARR[] = { "2. Abrir: http://192.168.4.1", "2. Open: http://192.168.4.1", "2. Ouvrir: http://192.168.4.1" };
const char* TXT_HELP_STEP3_ARR[] = { "3. Configurar WiFi, API Key, ubicacion", "3. Configure WiFi, API Key, location", "3. Configurer WiFi, API Key, emplacement" };
const char* TXT_HELP_STEP4_ARR[] = { "4. Configurar demas parametros", "4. Configure other parameters", "4. Configurer autres parametres" };
const char* TXT_HELP_STEP5_ARR[] = { "5. Guardar y reiniciar", "5. Save and restart", "5. Enregistrer et redemarrer" };
const char* TXT_HELP_TROUBLE_ARR[] = { "Solucion de Problemas", "Troubleshooting", "Depannage" };
const char* TXT_HELP_NOWIFI_ARR[] = { "Sin WiFi: Verifica SSID y password", "No WiFi: Check SSID and password", "Sans WiFi: Verifier SSID et mot de passe" };
const char* TXT_HELP_NODATA_ARR[] = { "Sin datos: Verifica API Key", "No data: Check API Key", "Sans donnees: Verifier API Key" };
const char* TXT_HELP_GHOST_ARR[] = { "Ghosting: Usa boton Limpiar", "Ghosting: Use Clean button", "Ghosting: Utiliser bouton Nettoyer" };
const char* TXT_HELP_TOUCH_ARR[] = { "Touch falla: Limpia pantalla", "Touch fails: Clean screen", "Touch echoue: Nettoyer ecran" };
const char* TXT_HELP_BOOT_ARR[] = { "MODO BOOTLOADER", "BOOTLOADER MODE", "MODE BOOTLOADER" };
const char* TXT_HELP_BOOT1_ARR[] = { "1. Mantener BOOT presionado", "1. Hold BOOT pressed", "1. Maintenir BOOT appuye" };
const char* TXT_HELP_BOOT2_ARR[] = { "2. Presionar RST", "2. Press RST", "2. Appuyer RST" };
const char* TXT_HELP_BOOT3_ARR[] = { "3. Soltar RST, luego BOOT", "3. Release RST, then BOOT", "3. Relacher RST, puis BOOT" };
const char* TXT_HELP_BOOT4_ARR[] = { "4. Cargar firmware", "4. Upload firmware", "4. Charger firmware" };

#define TXT_HELP_NAV String(TXT_HELP_NAV_ARR[currentLang])
#define TXT_HELP_CALENDAR String(TXT_HELP_CALENDAR_ARR[currentLang])
#define TXT_HELP_ZONE String(TXT_HELP_ZONE_ARR[currentLang])
#define TXT_HELP_ICONS String(TXT_HELP_ICONS_ARR[currentLang])
#define TXT_HELP_GRAPH_LEFT String(TXT_HELP_GRAPH_LEFT_ARR[currentLang])
#define TXT_HELP_GRAPH_RIGHT String(TXT_HELP_GRAPH_RIGHT_ARR[currentLang])
#define TXT_HELP_BATTERY String(TXT_HELP_BATTERY_ARR[currentLang])
#define TXT_HELP_INITIAL String(TXT_HELP_INITIAL_ARR[currentLang])
#define TXT_HELP_STEP1 String(TXT_HELP_STEP1_ARR[currentLang])
#define TXT_HELP_STEP2 String(TXT_HELP_STEP2_ARR[currentLang])
#define TXT_HELP_STEP3 String(TXT_HELP_STEP3_ARR[currentLang])
#define TXT_HELP_STEP4 String(TXT_HELP_STEP4_ARR[currentLang])
#define TXT_HELP_STEP5 String(TXT_HELP_STEP5_ARR[currentLang])
#define TXT_HELP_TROUBLE String(TXT_HELP_TROUBLE_ARR[currentLang])
#define TXT_HELP_NOWIFI String(TXT_HELP_NOWIFI_ARR[currentLang])
#define TXT_HELP_NODATA String(TXT_HELP_NODATA_ARR[currentLang])
#define TXT_HELP_GHOST String(TXT_HELP_GHOST_ARR[currentLang])
#define TXT_HELP_TOUCH String(TXT_HELP_TOUCH_ARR[currentLang])
#define TXT_HELP_BOOT String(TXT_HELP_BOOT_ARR[currentLang])
#define TXT_HELP_BOOT1 String(TXT_HELP_BOOT1_ARR[currentLang])
#define TXT_HELP_BOOT2 String(TXT_HELP_BOOT2_ARR[currentLang])
#define TXT_HELP_BOOT3 String(TXT_HELP_BOOT3_ARR[currentLang])
#define TXT_HELP_BOOT4 String(TXT_HELP_BOOT4_ARR[currentLang])

#define TXT_BACK String(TXT_BACK_ARR[currentLang])
#define TXT_CURRENT_CONDITIONS String(TXT_CURRENT_CONDITIONS_ARR[currentLang])
#define TXT_5DAY_FORECAST String(TXT_5DAY_FORECAST_ARR[currentLang])
#define TXT_WEATHER_TRENDS String(TXT_WEATHER_TRENDS_ARR[currentLang])
#define TXT_NEXT_24H String(TXT_NEXT_24H_ARR[currentLang])
#define TXT_5DAY_OUTLOOK String(TXT_5DAY_OUTLOOK_ARR[currentLang])
#define TXT_SYSTEM_INFO String(TXT_SYSTEM_INFO_ARR[currentLang])
#define TXT_INFO_HARDWARE String(TXT_INFO_HARDWARE_ARR[currentLang])
#define TXT_INFO_SOFTWARE String(TXT_INFO_SOFTWARE_ARR[currentLang])
#define TXT_INFO_HELP String(TXT_INFO_HELP_ARR[currentLang])
#define TXT_INFO_CREDITS String(TXT_INFO_CREDITS_ARR[currentLang])

// ============== HARDWARE SCREEN ==============
const char* TXT_HW_MICRO_ARR[] = { "Microcontrolador: ESP32-S3", "Microcontroller: ESP32-S3", "Microcontroleur: ESP32-S3" };
const char* TXT_HW_CPU_ARR[] = { "CPU: Dual-core Xtensa LX7 @ 240MHz", "CPU: Dual-core Xtensa LX7 @ 240MHz", "CPU: Dual-core Xtensa LX7 @ 240MHz" };
const char* TXT_HW_FLASH_ARR[] = { "Memoria Flash: 16 MB", "Flash Memory: 16 MB", "Memoire Flash: 16 Mo" };
const char* TXT_HW_PSRAM_ARR[] = { "PSRAM: 8 MB OPI", "PSRAM: 8 MB OPI", "PSRAM: 8 Mo OPI" };
const char* TXT_HW_WIFI_ARR[] = { "WiFi: 802.11 b/g/n 2.4GHz", "WiFi: 802.11 b/g/n 2.4GHz", "WiFi: 802.11 b/g/n 2.4GHz" };
const char* TXT_HW_BLE_ARR[] = { "Bluetooth: BLE 5.0 (disponible)", "Bluetooth: BLE 5.0 (available)", "Bluetooth: BLE 5.0 (disponible)" };
const char* TXT_HW_ACTIVE_ARR[] = { "Consumo Activo: ~150mA", "Active Power: ~150mA", "Conso. Actif: ~150mA" };
const char* TXT_HW_SLEEP_ARR[] = { "Consumo Deep Sleep: ~10uA", "Deep Sleep Power: ~10uA", "Conso. Veille: ~10uA" };
const char* TXT_HW_DISPLAY_ARR[] = { "Pantalla: E-Paper 4.7\"", "Display: E-Paper 4.7\"", "Ecran: E-Paper 4.7\"" };
const char* TXT_HW_RES_ARR[] = { "Resolucion: 960 x 540 px", "Resolution: 960 x 540 px", "Resolution: 960 x 540 px" };
const char* TXT_HW_COLORS_ARR[] = { "Colores: 16 niveles gris", "Colors: 16 grayscale levels", "Couleurs: 16 niveaux gris" };
const char* TXT_HW_REFRESH_ARR[] = { "Tiempo refresco: ~0.5 seg", "Refresh time: ~0.5 sec", "Temps raffraich.: ~0.5 sec" };
const char* TXT_HW_ANGLE_ARR[] = { "Angulo vision: ~180 grados", "Viewing angle: ~180 degrees", "Angle de vue: ~180 degres" };
const char* TXT_HW_TOUCH_ARR[] = { "Touch: GT911 Capacitivo I2C", "Touch: GT911 Capacitive I2C", "Touch: GT911 Capacitif I2C" };
const char* TXT_HW_BATT_ARR[] = { "Bateria: LiPo 3.7V (3.2-4.2V)", "Battery: LiPo 3.7V (3.2-4.2V)", "Batterie: LiPo 3.7V (3.2-4.2V)" };
const char* TXT_HW_SD_ARR[] = { "MicroSD: SPI FAT32/exFAT", "MicroSD: SPI FAT32/exFAT", "MicroSD: SPI FAT32/exFAT" };

#define TXT_HW_MICRO String(TXT_HW_MICRO_ARR[currentLang])
#define TXT_HW_CPU String(TXT_HW_CPU_ARR[currentLang])
#define TXT_HW_FLASH String(TXT_HW_FLASH_ARR[currentLang])
#define TXT_HW_PSRAM String(TXT_HW_PSRAM_ARR[currentLang])
#define TXT_HW_WIFI String(TXT_HW_WIFI_ARR[currentLang])
#define TXT_HW_BLE String(TXT_HW_BLE_ARR[currentLang])
#define TXT_HW_ACTIVE String(TXT_HW_ACTIVE_ARR[currentLang])
#define TXT_HW_SLEEP String(TXT_HW_SLEEP_ARR[currentLang])
#define TXT_HW_DISPLAY String(TXT_HW_DISPLAY_ARR[currentLang])
#define TXT_HW_RES String(TXT_HW_RES_ARR[currentLang])
#define TXT_HW_COLORS String(TXT_HW_COLORS_ARR[currentLang])
#define TXT_HW_REFRESH String(TXT_HW_REFRESH_ARR[currentLang])
#define TXT_HW_ANGLE String(TXT_HW_ANGLE_ARR[currentLang])
#define TXT_HW_TOUCH String(TXT_HW_TOUCH_ARR[currentLang])
#define TXT_HW_BATT String(TXT_HW_BATT_ARR[currentLang])
#define TXT_HW_SD String(TXT_HW_SD_ARR[currentLang])

// ============== SOFTWARE SCREEN ==============
const char* TXT_SW_SCREENS_ARR[] = { "Pantallas: 13 navegables", "Screens: 13 navigable", "Ecrans: 13 navigables" };
const char* TXT_SW_MULTIWIFI_ARR[] = { "Multi-WiFi: Hasta 3 redes", "Multi-WiFi: Up to 3 networks", "Multi-WiFi: Jusqu'a 3 reseaux" };
const char* TXT_SW_HISTSD_ARR[] = { "Historial SD: ~1 ano de datos", "SD History: ~1 year of data", "Historique SD: ~1 an de donnees" };
const char* TXT_SW_HISTINT_ARR[] = { "Historial Int: ~7 dias (FFat)", "Int. History: ~7 days (FFat)", "Historique Int: ~7 jours (FFat)" };
const char* TXT_SW_UPDATE_ARR[] = { "Actualizacion: 5-120 min config.", "Update: 5-120 min configurable", "Mise a jour: 5-120 min config." };
const char* TXT_SW_LANGS_ARR[] = { "Idiomas: ES / EN / FR", "Languages: ES / EN / FR", "Langues: ES / EN / FR" };
const char* TXT_SW_APMODE_ARR[] = { "Modo AP: WeatherStation-Setup", "AP Mode: WeatherStation-Setup", "Mode AP: WeatherStation-Setup" };
const char* TXT_SW_PORTAL_ARR[] = { "Portal: http://192.168.4.1", "Portal: http://192.168.4.1", "Portail: http://192.168.4.1" };
const char* TXT_SW_PASS_ARR[] = { "Password: weather123", "Password: weather123", "Mot de passe: weather123" };
const char* TXT_SW_API_ARR[] = { "API: WeatherAPI.com", "API: WeatherAPI.com", "API: WeatherAPI.com" };
const char* TXT_SW_ENDPOINT_ARR[] = { "Endpoint: /v1/forecast.json", "Endpoint: /v1/forecast.json", "Endpoint: /v1/forecast.json" };
const char* TXT_SW_FORECAST_ARR[] = { "Pronostico: 3 dias", "Forecast: 3 days", "Previsions: 3 jours" };
const char* TXT_SW_UVAQI_ARR[] = { "UV/AQI: Incluido", "UV/AQI: Included", "UV/IQA: Inclus" };
const char* TXT_SW_ONECALL_ARR[] = { "Una llamada por actualizacion", "One call per update", "Un appel par mise a jour" };
const char* TXT_SW_LIMIT_ARR[] = { "Limite gratis: 1M llamadas/mes", "Free limit: 1M calls/month", "Limite gratuit: 1M appels/mois" };
const char* TXT_SW_STORAGE_ARR[] = { "Almacenamiento: NVS Preferences", "Storage: NVS Preferences", "Stockage: NVS Preferences" };

#define TXT_SW_SCREENS String(TXT_SW_SCREENS_ARR[currentLang])
#define TXT_SW_MULTIWIFI String(TXT_SW_MULTIWIFI_ARR[currentLang])
#define TXT_SW_HISTSD String(TXT_SW_HISTSD_ARR[currentLang])
#define TXT_SW_HISTINT String(TXT_SW_HISTINT_ARR[currentLang])
#define TXT_SW_UPDATE String(TXT_SW_UPDATE_ARR[currentLang])
#define TXT_SW_LANGS String(TXT_SW_LANGS_ARR[currentLang])
#define TXT_SW_APMODE String(TXT_SW_APMODE_ARR[currentLang])
#define TXT_SW_PORTAL String(TXT_SW_PORTAL_ARR[currentLang])
#define TXT_SW_PASS String(TXT_SW_PASS_ARR[currentLang])
#define TXT_SW_API String(TXT_SW_API_ARR[currentLang])
#define TXT_SW_ENDPOINT String(TXT_SW_ENDPOINT_ARR[currentLang])
#define TXT_SW_FORECAST String(TXT_SW_FORECAST_ARR[currentLang])
#define TXT_SW_UVAQI String(TXT_SW_UVAQI_ARR[currentLang])
#define TXT_SW_ONECALL String(TXT_SW_ONECALL_ARR[currentLang])
#define TXT_SW_LIMIT String(TXT_SW_LIMIT_ARR[currentLang])
#define TXT_SW_STORAGE String(TXT_SW_STORAGE_ARR[currentLang])

// ============== CREDITS SCREEN ==============
const char* TXT_CR_AUTHOR_ARR[] = { "Autor Original", "Original Author", "Auteur Original" };
const char* TXT_CR_ADAPTATIONS_ARR[] = { "Adaptaciones", "Adaptations", "Adaptations" };
const char* TXT_CR_ADAPT1_ARR[] = { "markbirss - Adaptacion LilyGo EPD", "markbirss - LilyGo EPD adaptation", "markbirss - Adaptation LilyGo EPD" };
const char* TXT_CR_ADAPT2_ARR[] = { "Xinyuan-LilyGO - Fork oficial", "Xinyuan-LilyGO - Official fork", "Xinyuan-LilyGO - Fork officiel" };
const char* TXT_CR_ADAPT3_ARR[] = { "Stefan Maetschke 2025 - makerguides.com", "Stefan Maetschke 2025 - makerguides.com", "Stefan Maetschke 2025 - makerguides.com" };
const char* TXT_CR_MODS_ARR[] = { "Modificaciones XE1E 2026", "XE1E Modifications 2026", "Modifications XE1E 2026" };
const char* TXT_CR_MOD1_ARR[] = { "* Navegacion tactil 13 pantallas", "* Touch navigation 13 screens", "* Navigation tactile 13 ecrans" };
const char* TXT_CR_MOD2_ARR[] = { "* Multi-idioma ES/EN/FR", "* Multi-language ES/EN/FR", "* Multi-langue ES/EN/FR" };
const char* TXT_CR_MOD3_ARR[] = { "* Portal cautivo y config web", "* Captive portal and web config", "* Portail captif et config web" };
const char* TXT_CR_MOD4_ARR[] = { "* Historial SD Card + FFat", "* SD Card + FFat history", "* Historique SD Card + FFat" };
const char* TXT_CR_MOD5_ARR[] = { "* UV Index y Calidad del Aire", "* UV Index and Air Quality", "* Indice UV et Qualite de l'Air" };
const char* TXT_CR_MOD6_ARR[] = { "* Calendario mensual y anual", "* Monthly and yearly calendar", "* Calendrier mensuel et annuel" };
const char* TXT_CR_HW_ARR[] = { "Hardware", "Hardware", "Materiel" };
const char* TXT_CR_LIBS_ARR[] = { "Librerias", "Libraries", "Bibliotheques" };
const char* TXT_CR_LINKS_ARR[] = { "Enlaces", "Links", "Liens" };

#define TXT_CR_AUTHOR String(TXT_CR_AUTHOR_ARR[currentLang])
#define TXT_CR_ADAPTATIONS String(TXT_CR_ADAPTATIONS_ARR[currentLang])
#define TXT_CR_ADAPT1 String(TXT_CR_ADAPT1_ARR[currentLang])
#define TXT_CR_ADAPT2 String(TXT_CR_ADAPT2_ARR[currentLang])
#define TXT_CR_ADAPT3 String(TXT_CR_ADAPT3_ARR[currentLang])
#define TXT_CR_MODS String(TXT_CR_MODS_ARR[currentLang])
#define TXT_CR_MOD1 String(TXT_CR_MOD1_ARR[currentLang])
#define TXT_CR_MOD2 String(TXT_CR_MOD2_ARR[currentLang])
#define TXT_CR_MOD3 String(TXT_CR_MOD3_ARR[currentLang])
#define TXT_CR_MOD4 String(TXT_CR_MOD4_ARR[currentLang])
#define TXT_CR_MOD5 String(TXT_CR_MOD5_ARR[currentLang])
#define TXT_CR_MOD6 String(TXT_CR_MOD6_ARR[currentLang])
#define TXT_CR_HW String(TXT_CR_HW_ARR[currentLang])
#define TXT_CR_LIBS String(TXT_CR_LIBS_ARR[currentLang])
#define TXT_CR_LINKS String(TXT_CR_LINKS_ARR[currentLang])

// ============== UPDATE INFO SCREEN ==============
const char* TXT_INFO_UPDATE_ARR[] = { "INFORMACION DE ACTUALIZACION", "UPDATE INFORMATION", "INFORMATIONS DE MISE A JOUR" };
const char* TXT_UPD_STATION_ARR[] = { "Informacion de Estacion", "Station Information", "Informations Station" };
const char* TXT_UPD_LAST_UPDATE_ARR[] = { "Ultima actualizacion:", "Last update:", "Derniere mise a jour:" };
const char* TXT_UPD_STATION_ID_ARR[] = { "ID Estacion:", "Station ID:", "ID Station:" };
const char* TXT_UPD_STATION_NAME_ARR[] = { "Nombre estacion:", "Station name:", "Nom station:" };
const char* TXT_UPD_COORDINATES_ARR[] = { "Coordenadas:", "Coordinates:", "Coordonnees:" };
const char* TXT_UPD_DATA_TIME_ARR[] = { "Datos de estacion:", "Station data:", "Donnees station:" };
const char* TXT_UPD_NETWORK_ARR[] = { "Informacion de Red", "Network Information", "Informations Reseau" };
const char* TXT_UPD_WIFI_NAME_ARR[] = { "Red WiFi:", "WiFi Network:", "Reseau WiFi:" };
const char* TXT_UPD_SIGNAL_ARR[] = { "Nivel senal:", "Signal level:", "Niveau signal:" };
const char* TXT_UPD_IP_ARR[] = { "Direccion IP:", "IP Address:", "Adresse IP:" };

#define TXT_INFO_UPDATE String(TXT_INFO_UPDATE_ARR[currentLang])
#define TXT_UPD_STATION String(TXT_UPD_STATION_ARR[currentLang])
#define TXT_UPD_LAST_UPDATE String(TXT_UPD_LAST_UPDATE_ARR[currentLang])
#define TXT_UPD_STATION_ID String(TXT_UPD_STATION_ID_ARR[currentLang])
#define TXT_UPD_STATION_NAME String(TXT_UPD_STATION_NAME_ARR[currentLang])
#define TXT_UPD_COORDINATES String(TXT_UPD_COORDINATES_ARR[currentLang])
#define TXT_UPD_DATA_TIME String(TXT_UPD_DATA_TIME_ARR[currentLang])
#define TXT_UPD_NETWORK String(TXT_UPD_NETWORK_ARR[currentLang])
#define TXT_UPD_WIFI_NAME String(TXT_UPD_WIFI_NAME_ARR[currentLang])
#define TXT_UPD_SIGNAL String(TXT_UPD_SIGNAL_ARR[currentLang])
#define TXT_UPD_IP String(TXT_UPD_IP_ARR[currentLang])

// ============== DISPLAY LABELS ==============
const char* TXT_HUMIDITY_ARR[] = { "Humedad", "Humidity", "Humidite" };
const char* TXT_PRESSURE_ARR[] = { "Presion", "Pressure", "Pression" };
const char* TXT_VISIBILITY_ARR[] = { "Visibilidad", "Visibility", "Visibilite" };
const char* TXT_CLOUDINESS_ARR[] = { "Nubosidad", "Cloudiness", "Nebulosite" };
const char* TXT_WIND_ARR[] = { "Viento", "Wind", "Vent" };
const char* TXT_WIND_DIR_ARR[] = { "Direccion Viento", "Wind Direction", "Direction Vent" };
const char* TXT_GUSTS_ARR[] = { "Rafagas", "Gusts", "Rafales" };
const char* TXT_RAIN_PROB_ARR[] = { "Prob. Lluvia", "Rain Prob.", "Prob. Pluie" };
const char* TXT_RAIN_24H_ARR[] = { "Lluvia 24h", "Rain 24h", "Pluie 24h" };
const char* TXT_RAIN_TODAY_ARR[] = { "Lluvia Hoy", "Rain Today", "Pluie Jour" };
const char* TXT_FEELS_LIKE_ARR[] = { "Sensacion termica", "Feels like", "Ressenti" };
const char* TXT_DEWPOINT_ARR[] = { "Punto de rocio", "Dew Point", "Point de rosee" };
const char* TXT_UV_INDEX_ARR[] = { "Indice UV", "UV Index", "Indice UV" };
const char* TXT_AIR_QUALITY_ARR[] = { "Calidad del Aire", "Air Quality", "Qualite de l'Air" };
const char* TXT_AQI_ARR[] = { "ICA", "AQI", "IQA" };
const char* TXT_AQI_GOOD_ARR[] = { "Bueno", "Good", "Bon" };
const char* TXT_AQI_FAIR_ARR[] = { "Aceptable", "Fair", "Acceptable" };
const char* TXT_AQI_MODERATE_ARR[] = { "Moderado", "Moderate", "Modere" };
const char* TXT_AQI_POOR_ARR[] = { "Malo", "Poor", "Mauvais" };
const char* TXT_AQI_VERY_POOR_ARR[] = { "Muy Malo", "Very Poor", "Tres Mauvais" };
const char* TXT_UV_LOW_ARR[] = { "Bajo", "Low", "Bas" };
const char* TXT_UV_MODERATE_ARR[] = { "Moderado", "Moderate", "Modere" };
const char* TXT_UV_HIGH_ARR[] = { "Alto", "High", "Eleve" };
const char* TXT_UV_VERY_HIGH_ARR[] = { "Muy Alto", "Very High", "Tres Eleve" };
const char* TXT_UV_EXTREME_ARR[] = { "Extremo", "Extreme", "Extreme" };
const char* TXT_OZONE_ARR[] = { "Ozono", "Ozone", "Ozone" };
// Abbreviated UV levels for compact display
const char* TXT_UV_LOW_S_ARR[] = { "Bajo", "Low", "Bas" };
const char* TXT_UV_MOD_S_ARR[] = { "Mod.", "Mod.", "Mod." };
const char* TXT_UV_HIGH_S_ARR[] = { "Alto", "High", "Haut" };
const char* TXT_UV_VHIGH_S_ARR[] = { "M.Alto", "V.High", "T.Haut" };
const char* TXT_UV_EXT_S_ARR[] = { "Extr.", "Extr.", "Extr." };

#define TXT_HUMIDITY String(TXT_HUMIDITY_ARR[currentLang])
#define TXT_PRESSURE String(TXT_PRESSURE_ARR[currentLang])
#define TXT_VISIBILITY String(TXT_VISIBILITY_ARR[currentLang])
#define TXT_CLOUDINESS String(TXT_CLOUDINESS_ARR[currentLang])
#define TXT_WIND String(TXT_WIND_ARR[currentLang])
#define TXT_WIND_DIR String(TXT_WIND_DIR_ARR[currentLang])
#define TXT_GUSTS String(TXT_GUSTS_ARR[currentLang])
#define TXT_RAIN_PROB String(TXT_RAIN_PROB_ARR[currentLang])
#define TXT_RAIN_24H String(TXT_RAIN_24H_ARR[currentLang])
#define TXT_RAIN_TODAY String(TXT_RAIN_TODAY_ARR[currentLang])
#define TXT_FEELS_LIKE String(TXT_FEELS_LIKE_ARR[currentLang])
#define TXT_DEWPOINT String(TXT_DEWPOINT_ARR[currentLang])
#define TXT_UV_INDEX String(TXT_UV_INDEX_ARR[currentLang])
#define TXT_AIR_QUALITY String(TXT_AIR_QUALITY_ARR[currentLang])
#define TXT_AQI String(TXT_AQI_ARR[currentLang])
#define TXT_AQI_GOOD String(TXT_AQI_GOOD_ARR[currentLang])
#define TXT_AQI_FAIR String(TXT_AQI_FAIR_ARR[currentLang])
#define TXT_AQI_MODERATE String(TXT_AQI_MODERATE_ARR[currentLang])
#define TXT_AQI_POOR String(TXT_AQI_POOR_ARR[currentLang])
#define TXT_AQI_VERY_POOR String(TXT_AQI_VERY_POOR_ARR[currentLang])
#define TXT_UV_LOW String(TXT_UV_LOW_ARR[currentLang])
#define TXT_UV_MODERATE String(TXT_UV_MODERATE_ARR[currentLang])
#define TXT_UV_HIGH String(TXT_UV_HIGH_ARR[currentLang])
#define TXT_UV_VERY_HIGH String(TXT_UV_VERY_HIGH_ARR[currentLang])
#define TXT_UV_EXTREME String(TXT_UV_EXTREME_ARR[currentLang])
#define TXT_OZONE String(TXT_OZONE_ARR[currentLang])
#define TXT_UV_LOW_S String(TXT_UV_LOW_S_ARR[currentLang])
#define TXT_UV_MOD_S String(TXT_UV_MOD_S_ARR[currentLang])
#define TXT_UV_HIGH_S String(TXT_UV_HIGH_S_ARR[currentLang])
#define TXT_UV_VHIGH_S String(TXT_UV_VHIGH_S_ARR[currentLang])
#define TXT_UV_EXT_S String(TXT_UV_EXT_S_ARR[currentLang])

// ============== INITIALIZE LANGUAGE ==============
void initLanguage(const char* lang) {
  setLanguage(lang);
  updateWeekdayPointer();
  updateMonthPointer();
}

// ============== WEATHER CONDITION SHORT DESCRIPTIONS ==============
// Maps WeatherAPI condition codes to short descriptions
String getConditionShortText(int code) {
  switch(code) {
    case 1000: { const char* t[] = {"Soleado", "Sunny", "Ensoleille"}; return t[currentLang]; }
    case 1003: { const char* t[] = {"Parcialmente nublado", "Partly cloudy", "Partiellement nuageux"}; return t[currentLang]; }
    case 1006: { const char* t[] = {"Nublado", "Cloudy", "Nuageux"}; return t[currentLang]; }
    case 1009: { const char* t[] = {"Cubierto", "Overcast", "Couvert"}; return t[currentLang]; }
    case 1030: { const char* t[] = {"Neblina", "Mist", "Brume"}; return t[currentLang]; }
    case 1063: { const char* t[] = {"Lluvia posible", "Rain possible", "Pluie possible"}; return t[currentLang]; }
    case 1066: { const char* t[] = {"Nieve posible", "Snow possible", "Neige possible"}; return t[currentLang]; }
    case 1069: { const char* t[] = {"Aguanieve posible", "Sleet possible", "Gresil possible"}; return t[currentLang]; }
    case 1072: { const char* t[] = {"Llovizna helada", "Freezing drizzle", "Bruine verglacante"}; return t[currentLang]; }
    case 1087: { const char* t[] = {"Tormenta electrica", "Thunderstorm", "Orage"}; return t[currentLang]; }
    case 1114: { const char* t[] = {"Ventisca", "Blowing snow", "Poudrerie"}; return t[currentLang]; }
    case 1117: { const char* t[] = {"Nevada intensa", "Blizzard", "Blizzard"}; return t[currentLang]; }
    case 1135: { const char* t[] = {"Niebla", "Fog", "Brouillard"}; return t[currentLang]; }
    case 1147: { const char* t[] = {"Niebla helada", "Freezing fog", "Brouillard givrant"}; return t[currentLang]; }
    case 1150: { const char* t[] = {"Llovizna ligera", "Light drizzle", "Bruine legere"}; return t[currentLang]; }
    case 1153: { const char* t[] = {"Llovizna", "Drizzle", "Bruine"}; return t[currentLang]; }
    case 1168: { const char* t[] = {"Llovizna helada", "Freezing drizzle", "Bruine verglacante"}; return t[currentLang]; }
    case 1171: { const char* t[] = {"Llovizna fuerte", "Heavy drizzle", "Forte bruine"}; return t[currentLang]; }
    case 1180: { const char* t[] = {"Lluvia ligera", "Light rain", "Pluie legere"}; return t[currentLang]; }
    case 1183: { const char* t[] = {"Lluvia ligera", "Light rain", "Pluie legere"}; return t[currentLang]; }
    case 1186: { const char* t[] = {"Lluvia moderada", "Moderate rain", "Pluie moderee"}; return t[currentLang]; }
    case 1189: { const char* t[] = {"Lluvia moderada", "Moderate rain", "Pluie moderee"}; return t[currentLang]; }
    case 1192: { const char* t[] = {"Lluvia fuerte", "Heavy rain", "Forte pluie"}; return t[currentLang]; }
    case 1195: { const char* t[] = {"Lluvia intensa", "Intense rain", "Pluie intense"}; return t[currentLang]; }
    case 1198: { const char* t[] = {"Lluvia helada", "Freezing rain", "Pluie verglacante"}; return t[currentLang]; }
    case 1201: { const char* t[] = {"Lluvia helada", "Freezing rain", "Pluie verglacante"}; return t[currentLang]; }
    case 1204: { const char* t[] = {"Aguanieve ligera", "Light sleet", "Gresil leger"}; return t[currentLang]; }
    case 1207: { const char* t[] = {"Aguanieve moderada", "Moderate sleet", "Gresil modere"}; return t[currentLang]; }
    case 1210: { const char* t[] = {"Nieve ligera", "Light snow", "Neige legere"}; return t[currentLang]; }
    case 1213: { const char* t[] = {"Nieve ligera", "Light snow", "Neige legere"}; return t[currentLang]; }
    case 1216: { const char* t[] = {"Nieve moderada", "Moderate snow", "Neige moderee"}; return t[currentLang]; }
    case 1219: { const char* t[] = {"Nieve moderada", "Moderate snow", "Neige moderee"}; return t[currentLang]; }
    case 1222: { const char* t[] = {"Nieve fuerte", "Heavy snow", "Forte neige"}; return t[currentLang]; }
    case 1225: { const char* t[] = {"Nieve intensa", "Intense snow", "Neige intense"}; return t[currentLang]; }
    case 1237: { const char* t[] = {"Granizo", "Ice pellets", "Grelons"}; return t[currentLang]; }
    case 1240: { const char* t[] = {"Chubascos ligeros", "Light showers", "Averses legeres"}; return t[currentLang]; }
    case 1243: { const char* t[] = {"Chubascos moderados", "Moderate showers", "Averses moderees"}; return t[currentLang]; }
    case 1246: { const char* t[] = {"Chubascos fuertes", "Heavy showers", "Fortes averses"}; return t[currentLang]; }
    case 1249: { const char* t[] = {"Aguanieve ligera", "Light sleet", "Gresil leger"}; return t[currentLang]; }
    case 1252: { const char* t[] = {"Aguanieve moderada", "Moderate sleet", "Gresil modere"}; return t[currentLang]; }
    case 1255: { const char* t[] = {"Nieve ligera", "Light snow", "Neige legere"}; return t[currentLang]; }
    case 1258: { const char* t[] = {"Nieve moderada", "Moderate snow", "Neige moderee"}; return t[currentLang]; }
    case 1261: { const char* t[] = {"Granizo ligero", "Light ice", "Grele legere"}; return t[currentLang]; }
    case 1264: { const char* t[] = {"Granizo moderado", "Moderate ice", "Grele moderee"}; return t[currentLang]; }
    case 1273: { const char* t[] = {"Tormenta electrica con lluvia", "Thunderstorm with rain", "Orage avec pluie"}; return t[currentLang]; }
    case 1276: { const char* t[] = {"Tormenta electrica fuerte", "Heavy thunderstorm", "Orage violent"}; return t[currentLang]; }
    case 1279: { const char* t[] = {"Tormenta electrica con nieve", "Thunderstorm with snow", "Orage avec neige"}; return t[currentLang]; }
    case 1282: { const char* t[] = {"Tormenta electrica fuerte", "Heavy thunderstorm", "Orage violent"}; return t[currentLang]; }
    default: { const char* t[] = {"Desconocido", "Unknown", "Inconnu"}; return t[currentLang]; }
  }
}

#endif // LANG_H
