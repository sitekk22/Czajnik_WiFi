#include "secrets.h"
#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>
#include <esp_check.h>
#include <esp_wifi.h>

#include <DallasTemperature.h>
#include <OneWire.h>

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;


WebServer server(80);
String header;

const byte relay_pin = 17;
const byte one_wire_bus = 25;
OneWire one_wire(one_wire_bus);
DallasTemperature sensors(&one_wire);

int led_pin = 16;
uint8_t target_temp = 0;

String webpage;

void toggleLED() { digitalWrite(led_pin, !digitalRead(led_pin)); }

void initWifi() {

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, pass);
  esp_wifi_set_ps(WIFI_PS_NONE);
  toggleLED();
}

String updateWebpage(uint8_t t) {

  webpage = "<!DOCTYPE html>\n";
  webpage += "<html lang=\"en\">\n";
  webpage += "<head>\n";
  webpage += "<meta charset=\"UTF-8\">\n";
  webpage += "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n";
  webpage += "<meta name=\"viewport\" content=\"width=device-width, "
             "initial-scale=1.0\">\n";
  webpage += "<title>Czajnik</title>\n";
  webpage += "<style>html {font-family: Hevletica; display: inline-block; "
             "margin: 0px auto; text-align: center;}\n";
  webpage += "body{margin-top: 50px;} h1 {color: #444444; margin: 50px  auto "
             "30px;} h3 {color: #444444; margin-bottom: 50px;}\n";

  webpage += "#button-container { text-align: center;}\n";

  webpage += ".ubutton{display: block; width: 20vw; height: 5vh; font-size: "
             "1.5rem; background-color: #66CDAA;\n";
  webpage += "border: none ;color: white; text-decoration: none; margin-left: "
             "35vw; margin-right: 35vw; ";
  webpage +=
      "margin-top: 3vh; margin-bottom: 3vh; min-width: 30vw; min-height: 6vh;";

  webpage += "border-radius: 6px; transition-duration: 0.4s;}\n";
  webpage += ".ubutton:hover { background-color: #5cc29f }\n";
  webpage += "</style>\n";

  webpage += "</head>\n";
  webpage += "<body>\n";
  webpage += "<h1>PRO CZAJNIK</h1>\n";
  webpage += "<h2> docelowa temperatura: ";
  webpage += t;
  webpage += "°C</h2> \n";
  webpage += "<h2> aktualna temperatura: ";
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);
  webpage += temperatureC;
  webpage += "<div id=\"button-container\">\n";

  webpage += "<form action=\"/70\">";
  webpage += "<input class = \"ubutton\" type=\"submit\" value=\"70°C\">";
  webpage += "</form>\n";

  webpage += "<form action=\"/75\">";
  webpage += "<input class = \"ubutton\" type=\"submit\" value=\"75°C\">";
  webpage += "</form>\n";

  webpage += "<form action=\"/80\">";
  webpage += "<input class = \"ubutton\" type=\"submit\" value=\"80°C\">";
  webpage += "</form>\n";

  webpage += "<form action=\"/85\">";
  webpage += "<input class = \"ubutton\" type=\"submit\" value=\"85°C\">";
  webpage += "</form>\n";

  webpage += "<form action=\"/90\">";
  webpage += "<input class = \"ubutton\" type=\"submit\" value=\"90°C\">";
  webpage += "</form>\n";

  webpage += "<form action=\"/95\">";
  webpage += "<input class = \"ubutton\" type=\"submit\" value=\"95°C\">";
  webpage += "</form>\n";

  webpage += "</div>\n";
  webpage += "</body>\n";
  webpage += "</html>\n";

  return webpage;
}

void updateTemperature(uint8_t t) {
  target_temp = t;
  toggleLED();
}
void redirect(uint8_t t) {

  updateTemperature(t);

  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}
void setup() {
  pinMode(led_pin, OUTPUT);
  pinMode(relay_pin, OUTPUT);
  digitalWrite(relay_pin, LOW);
  Serial.begin(115200);
  Serial.println();
  initWifi();

  server.on(
      "/", []() { server.send(200, "text/html", updateWebpage(target_temp)); });
  server.on("/70", []() { redirect(70); });
  server.on("/75", []() { redirect(75); });
  server.on("/80", []() { redirect(80); });
  server.on("/85", []() { redirect(85); });
  server.on("/90", []() { redirect(90); });
  server.on("/95", []() { redirect(95); });
  server.begin();
}

short korekta = 0;
void switch_relay(float tempC) {
  if (tempC < 60){
    if (target_temp >= 85) {
      korekta = 7;
    } else if ( target_temp < 85 && target_temp >= 75) {
      korekta = 5;
    } else {
      korekta = 3;
    }
  } else { korekta = 3;}

  if (tempC <= target_temp - korekta && tempC != -127 && target_temp != 0) {
    digitalWrite(relay_pin, HIGH);
    Serial.println("Relay ON");
    delay(100);

  } else if (tempC > target_temp - korekta) {
    digitalWrite(relay_pin, LOW);
    target_temp = 0;
    delay(100);

  }

  else {
    if (tempC != -127) {
      digitalWrite(relay_pin, LOW);
      Serial.println("Relay OFF");
    }
  }
}

void loop() {
  server.handleClient();
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);
  Serial.println(temperatureC);
  switch_relay(temperatureC);
  delay(100);
}