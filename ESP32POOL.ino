// Type de carte Arduino IDE: GENERIC ESP8266 Module 4MB

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <OneWire.h> 
#include <DallasTemperature.h>

#define OTAName                    "ESP32POOL"
#define WifiSSID                   "TBSOFT"
#define WifiPass                   "TB04011966"
#define IP                         52

#define wifiMaxInterval            60000
#define pwmFreq                    20             
#define tempCorrector              1

// I=>INPUT O=>OUTPUT PWM=>analogWrite OW=>OneWire(resistance 4.7K de pullUp sur Data)                                     
#define rx                         3  // I/O/PWM/OWko HIGH at BOOT
#define tx                         1  // I/O/PWM/OWko HIGH at BOOT boot fails if pulled LOW
#define P0                         16 // I/O/PWM/OWko HIGH at BOOT INPUT -> necessite PullUp externe
#define P1                         5  // I/O/PWM/OW   often used as SCL (I2C)
#define P2                         4  // I/O/PWM/OW   often used as SDA (I2C)
#define P3                         0  // I/O/PWM/OW   HIGH at BOOT boot fails if pulled LOW > FLASH
#define P4                         2  // I/O/PWM/OW   boot fails if pulled LOW
#define P5                         14 // I/O/PWM/OW  
#define P6                         12 // I/O/PWM/OW  
#define P7                         13 // I/O/PWM/OW  
#define P8                         15 // I/O/PWM/OW   boot fails if pulled HIGH  INPUT -> necessite PullDown externe

#define LIBRE1                     tx
#define LIBRE2                     rx
#define LIBRE3                     P0
#define ONE_WIRE_BUS               P1
#define LIBRE4                     P2             
#define LIBRE5                     P3
#define SSR                        P4
#define SPOT                       P5
#define NCC                        P6  
#define CIRCU                      P7 
#define FILTR                      P8

float alpha                        = 0;
float poolTemp                     = 0;

unsigned long wifiTimer            = millis();

ESP8266WebServer server(80); OneWire oneWire(ONE_WIRE_BUS); DallasTemperature sensors(&oneWire, ONE_WIRE_BUS);

void loop() { 
  server.handleClient(); ArduinoOTA.handle(); 
  if (millis() - wifiTimer > wifiMaxInterval) { alpha = 0; wifiTimer = millis(); }
}
void root() { 
  if (server.arg("alpha" )!="") { 
    alpha = server.arg("alpha").toFloat(); 
    analogWrite(SSR, alpha*255);
  }  
  poolTemp=sensors.getTempCByIndex(0)*tempCorrector; 
  sensors.requestTemperatures();  
  sendJsonResponse(); 
  wifiTimer = millis(); 
}
void sendJsonResponse() { 
  setHeaders(); server.send(200, "application/json", "{ "
  "\"api\": \"ecsPoolStatus\", "
  "\"poolTemp\": " + String(poolTemp, 1) + ", "
  "\"SPOT\": " + ((digitalRead(SPOT)==HIGH)? "true":"false") + ", "  
  "\"NCC\": " + ((digitalRead(NCC)==HIGH)? "true":"false") + ", "  
  "\"CIRCU\": " + ((digitalRead(CIRCU)==HIGH)? "true":"false") + ", "                      
  "\"FILTR\": " + ((digitalRead(FILTR)==HIGH)? "true":"false") + ", "                   
  "\"alpha\": " + String(alpha, 6) + " }"); 
}
void cors() { setHeaders(); server.send(200, "text/plain", "" ); }
void handleNotFound() { if (server.method() == HTTP_OPTIONS) { setHeaders(); server.send(204); } else server.send(404, "text/plain", ""); }
void setHeaders() {
  server.sendHeader("Access-Control-Max-Age", "10000");
  server.sendHeader("Access-Control-Allow-Methods", "GET,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "*");  
  server.sendHeader("Access-Control-Allow-Origin","*");      
}
void setup() {
  pinMode(SPOT,  OUTPUT);   
  pinMode(SSR,   OUTPUT); 
  pinMode(NCC,   OUTPUT); 
  pinMode(CIRCU, OUTPUT); 
  pinMode(FILTR, OUTPUT); 
  digitalWrite(SPOT,  LOW);  
  digitalWrite(SSR,   LOW);
  digitalWrite(NCC,   LOW);  
  digitalWrite(CIRCU, LOW);  
  digitalWrite(FILTR, LOW);  

  analogWriteFreq(pwmFreq); analogWrite(SSR, 0);

  sensors.begin(); 
  sensors.setResolution(9); 
  sensors.setWaitForConversion(false);  
  sensors.requestTemperatures(); 
  
  WiFi.config(IPAddress(192, 168, 0, IP), IPAddress(192, 168, 0, 1), IPAddress(255, 255, 255, 0));
  WiFi.hostname(OTAName); WiFi.mode(WIFI_STA); WiFi.begin(WifiSSID, WifiPass);
  while (WiFi.status() != WL_CONNECTED) { delay(250); }

  ArduinoOTA.setHostname(OTAName); ArduinoOTA.begin();  

  server.on("/", HTTP_OPTIONS, cors);
  server.on("/", HTTP_GET, root);
  server.onNotFound(handleNotFound);  
  server.begin(); 
}

