# train-signal-tracker
🚂 Train Signal Tracker

Mapping GSM dead zones on train routes to enable seamless internet connectivity for passengers.

The Problem
Anyone who's traveled on Indian trains knows — internet just dies randomly. You're in a call, it drops. You're downloading something, it stops. Nobody knows where these dead zones are or how long they last.

The Idea
What if we could predict dead zones before the train hits them?
Map every dead zone on every route. Use that data to preconnect to the next tower before signal drops. Result — seamless internet on trains, no drops, no buffering.

This device is Phase 1 — the mapper.

What this does right now
ESP32 mounted on a train reads GPS coordinates + GSM signal strength every second and plots everything on a live dashboard. As the train moves, the map fills up with color coded signal zones — green where signal is strong, red where it dies.
Run it on enough routes, you have a complete dead zone map of Indian railways.
Hardware

ESP32 DEVKIT V1 (Dual core Xtensa LX6, 240MHz)
NEO-6M GPS module
SIM800L GSM module
DHT11 temperature + humidity sensor
18650 Li-ion battery (SIM800L power)

Architecture
Three FreeRTOS tasks running in parallel across ESP32's dual cores:

Core 0:
ReadGPS — reads NMEA sentences from NEO-6M via UART every second
ReadGSM — sends AT+CSQ to SIM800L every 5 seconds, parses signal strength

Core 1:
WebServer — serves live Leaflet.js dashboard, auto-refreshes every 5 seconds

Mutex synchronization prevents race conditions when tasks share data across cores.
Dashboard
Open ESP32's IP in any browser on same network:

Live map with current train location marker
Color coded signal circles as train moves

🟢 Green — strong (20-31)
🟠 Orange — medium (10-19)
🔴 Red — dead zone (0-9)


Signal strength card with progress bar
Temperature + humidity readings
Readings logged counter (up to 50)
Auto-refreshes every 5 seconds

Embedded Concepts Implemented
Communication Protocols:

UART — GPS and GSM both communicate via hardware UART (Serial1, Serial2)
AT Commands — SIM800L controlled via GSM AT command set
NMEA Parsing — raw GPS sentences parsed using TinyGPSPlus
HTTP — ESP32 acts as web server, serves HTML to browser clients

ESP32 Internals:

GPIO — sensor interfacing and pin multiplexing
ADC — analog sensor reading
PWM — LED control
WiFi — STA mode, IP assignment, RSSI reading
Dual core utilization — tasks distributed across Core 0 and Core 1

RTOS:

FreeRTOS tasks — three independent tasks running concurrently
Task pinning — GPS/GSM on Core 0, WebServer on Core 1
vTaskDelay — non-blocking cooperative scheduling
Mutex (xSemaphoreCreateMutex) — shared variable protection
Race condition prevention across cores

Interrupts and Timing:

Hardware interrupts — touchAttachInterrupt, capacitive touch sensing
IRAM_ATTR — ISR stored in RAM for fast execution
Volatile keyword — safe shared variable access between ISR and main code
millis() based non-blocking timing
Deep sleep with timer wakeup (esp_deep_sleep_start)

Web + Visualization:

WiFiServer — HTTP server hosted on ESP32
Leaflet.js — interactive OpenStreetMap integration
Dynamic HTML — sensor data injected into JavaScript
Glassmorphism UI — CSS backdrop-filter, gradient styling
Auto-refresh — meta http-equiv refresh

Software Concepts:

String parsing — AT response parsing using indexOf, substring, toInt
Struct based data logging — GPS + signal stored in RAM array
Signal validity checking — distinguishing valid CSQ from garbage
Non-blocking architecture — no delay() in production code

Libraries

TinyGPSPlus — Mikal Hart
DHT sensor library — Adafruit
WiFi.h — ESP32 built-in
FreeRTOS — ESP32 built-in

Roadmap

 GPS + GSM + DHT11 data collection
 FreeRTOS multi-task architecture
 Live Leaflet.js dashboard with auto-refresh
 Color coded dead zone mapping
 Mutex protected shared data
 Signal validity filtering
 SD card permanent data logging
 Drive test + dead zone database
 Predictive preconnection algorithm
 Multi-route dead zone aggregation
 Production hardware design

Demo
🎥 Field test video coming soon — drive test around Kanpur in progress.

Phase 1 of a larger idea — making train internet actually work in India.
Built during 3rd year ECE, IIIT Bhagalpur.
