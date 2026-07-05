#include <DHT.h>
#include <WiFi.h>
#include <TinyGPSPlus.h>

TinyGPSPlus gps;
WiFiServer server(80);
DHT dht(4,DHT11);
const char* ssid="AdityaKaPapa";
const char* pass="12345678";
String header="";
String s="";
unsigned long ct=millis();
unsigned long pt=0;
unsigned long to=5000;
String newline="";
SemaphoreHandle_t mutex;
int latestSignal=0;
bool signalValid=false;

struct Reading {
    float lat;
    float lng;
    int signal;
};
Reading readings[50];
int readingCount = 0;

void ReadGPS(void* parameter) {
    for(;;) {
        while(Serial2.available()>0) {
            gps.encode(Serial2.read());
        }
        Serial.println(String(gps.location.lat()));
        Serial.println(String(gps.location.lng()));
        
        if(gps.location.isValid() && readingCount < 50) {
            xSemaphoreTake(mutex, portMAX_DELAY);
            bool valid = signalValid;
            int sig = latestSignal;
            xSemaphoreGive(mutex);
            
            if(valid) {
                readings[readingCount].lat = gps.location.lat();
                readings[readingCount].lng = gps.location.lng();
                readings[readingCount].signal = sig;
                readingCount++;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void ReadGSM(void* parameter) {
    for(;;) {
        Serial1.println("AT+CSQ");
        vTaskDelay(pdMS_TO_TICKS(500));
        s = "";
        unsigned long start = millis();
        while(millis() - start < 2000) {
            if(Serial1.available()) {
                s += (char)Serial1.read();
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        xSemaphoreTake(mutex, portMAX_DELAY);
        if(s.indexOf("+CSQ:") >= 0) {
            latestSignal = (s.substring(s.indexOf(":")+2, s.indexOf(","))).toInt();
            signalValid = true;
        } else {
            signalValid = false;
        }
        Serial.println(s);
        xSemaphoreGive(mutex);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void WebServer(void* parameters) {
    for(;;) {
        WiFiClient client=server.available();
        if(client) {
            ct=millis();
            pt=ct;
            while(client.connected() && ct-pt<=to) {
                ct=millis();
                if(client.available()) {
                    char c=client.read();
                    header+=c;
                    if(c=='\n' && newline=="") {
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println("Connection:close");
                        client.println();
                        client.println("<!DOCTYPE html><html>");
                        client.println("<head>");
                        client.println("<meta http-equiv='refresh' content='5'>");
                        client.println("<link rel='stylesheet' href='https://unpkg.com/leaflet/dist/leaflet.css'/>");
                        client.println("<script src='https://unpkg.com/leaflet/dist/leaflet.js'></script>");
                        client.println("<style>");
                        client.println("* { margin: 0; padding: 0; box-sizing: border-box; }");
                        client.println("body { font-family: 'Segoe UI', sans-serif; min-height: 100vh; background: linear-gradient(135deg, #e0f7ff 0%, #c8e6ff 30%, #dcd6ff 60%, #f0e6ff 100%); color: #1a1a2e; padding: 24px; }");
                        client.println(".title { text-align: center; font-size: 30px; font-weight: 800; background: linear-gradient(90deg, #0077b6, #7b2ff7); -webkit-background-clip: text; -webkit-text-fill-color: transparent; margin-bottom: 6px; letter-spacing: 2px; }");
                        client.println(".subtitle { text-align: center; font-size: 12px; color: rgba(0,0,0,0.35); margin-bottom: 20px; letter-spacing: 3px; text-transform: uppercase; }");
                        client.println("#map { border-radius: 24px; margin-bottom: 24px; border: 1px solid rgba(255,255,255,0.8); box-shadow: 0 8px 32px rgba(0,119,182,0.15), inset 0 1px 0 rgba(255,255,255,0.9); }");
                        client.println(".cards { display: grid; grid-template-columns: repeat(2, 1fr); gap: 16px; }");
                        client.println(".card { background: rgba(255,255,255,0.45); backdrop-filter: blur(20px); -webkit-backdrop-filter: blur(20px); border: 1px solid rgba(255,255,255,0.75); border-radius: 20px; padding: 22px 24px; box-shadow: 0 4px 24px rgba(0,0,0,0.08), inset 0 1px 0 rgba(255,255,255,0.9); transition: transform 0.2s; }");
                        client.println(".card:hover { transform: translateY(-2px); }");
                        client.println(".card-label { font-size: 11px; color: rgba(0,0,0,0.4); letter-spacing: 2px; text-transform: uppercase; margin-bottom: 10px; font-weight: 600; }");
                        client.println(".card-value { font-size: 28px; font-weight: 700; background: linear-gradient(90deg, #0077b6, #7b2ff7); -webkit-background-clip: text; -webkit-text-fill-color: transparent; }");
                        client.println(".card-unit { font-size: 14px; color: rgba(0,0,0,0.3); margin-left: 3px; -webkit-text-fill-color: rgba(0,0,0,0.3); }");
                        client.println(".signal-bar { height: 6px; border-radius: 6px; margin-top: 12px; background: rgba(0,0,0,0.08); overflow: hidden; }");
                        client.println(".signal-fill { height: 100%; border-radius: 6px; background: linear-gradient(90deg, #0077b6, #7b2ff7); box-shadow: 0 0 8px rgba(123,47,247,0.4); }");
                        client.println(".dot { display: inline-block; width: 8px; height: 8px; border-radius: 50%; background: #00b894; margin-right: 6px; box-shadow: 0 0 6px #00b894; animation: pulse 1.5s infinite; }");
                        client.println("@keyframes pulse { 0%,100%{opacity:1;transform:scale(1)} 50%{opacity:0.5;transform:scale(1.3)} }");
                        client.println("</style>");
                        client.println("</head>");
                        client.println("<body>");
                        client.println("<div class='title'>Train Signal Tracker</div>");
                        client.println("<div class='subtitle'><span class='dot'></span>LIVE IoT DASHBOARD</div>");
                        client.println("<div id='map' style='height:400px'></div>");
                        client.println("<script>");
                        if(gps.location.isValid()) {
                            client.println("var map = L.map('map').setView([" + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6) + "], 13);");
                            client.println("L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png').addTo(map);");
                            client.println("L.marker([" + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6) + "]).addTo(map).bindPopup('Train Location').openPopup();");
                        } else {
                            client.println("var map = L.map('map').setView([26.431, 80.334], 13);");
                            client.println("L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png').addTo(map);");
                            client.println("L.marker([26.431, 80.334]).addTo(map).bindPopup('Waiting for GPS fix...').openPopup();");
                        }
                        for(int i = 0; i < readingCount; i++) {
                            String color;
                            if(readings[i].signal >= 20) color = "green";
                            else if(readings[i].signal >= 10) color = "orange";
                            else color = "red";
                            client.println("L.circle([" + String(readings[i].lat, 6) + "," + String(readings[i].lng, 6) + "], {color: '" + color + "', radius: 20}).addTo(map).bindPopup('Signal: " + String(readings[i].signal) + "/31');");
                        }
                        client.println("</script>");

                        xSemaphoreTake(mutex, portMAX_DELAY);
                        int sigVal = latestSignal;
                        bool valid = signalValid;
                        xSemaphoreGive(mutex);
                        String signal = valid ? String(sigVal) : "--";
                        int sigPercent = valid ? (sigVal * 100) / 31 : 0;

                        client.println("<div class='cards'>");
                        client.println("<div class='card'>");
                        client.println("<div class='card-label'>Signal Strength</div>");
                        client.println("<div class='card-value'>" + signal + "<span class='card-unit'>/31</span></div>");
                        client.println("<div class='signal-bar'><div class='signal-fill' style='width:" + String(sigPercent) + "%'></div></div>");
                        client.println("</div>");

                        client.println("<div class='card'>");
                        client.println("<div class='card-label'>Temperature</div>");
                        client.println("<div class='card-value'>" + String(dht.readTemperature(), 1) + "<span class='card-unit'>C</span></div>");
                        client.println("</div>");

                        client.println("<div class='card'>");
                        client.println("<div class='card-label'>Humidity</div>");
                        client.println("<div class='card-value'>" + String(dht.readHumidity(), 1) + "<span class='card-unit'>%</span></div>");
                        client.println("</div>");

                        client.println("<div class='card'>");
                        client.println("<div class='card-label'>Readings Logged</div>");
                        client.println("<div class='card-value'>" + String(readingCount) + "<span class='card-unit'>/50</span></div>");
                        client.println("</div>");

                        client.println("</div>");
                        client.println("</body></html>");
                        break;
                    }
                    else if(c=='\n') { newline=""; }
                    else if(c!='\r') { newline+=c; }
                }
            }
            header="";
            client.stop();
            Serial.println("Client disconnected");
            vTaskDelay(pdMS_TO_TICKS(3000));
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial2.begin(9600,SERIAL_8N1,16,17);
    Serial1.begin(9600,SERIAL_8N1,26,27);
    dht.begin();
    WiFi.begin(ssid,pass);
    Serial.println("Connecting..");
    while(WiFi.status()!=WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }
    Serial.println("Connected");
    Serial.println(WiFi.localIP());
    server.begin();
    mutex = xSemaphoreCreateMutex();
    xTaskCreatePinnedToCore(ReadGPS, "GPS READ", 10000, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(ReadGSM, "GSM READ", 10000, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(WebServer, "WEB SERVER", 10000, NULL, 1, NULL, 1);
}

void loop() {}