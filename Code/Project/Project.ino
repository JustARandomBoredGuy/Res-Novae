#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <FS.h>
#include <HX711_ADC.h>
#include <Wire.h>


//pins:
const int HX711_dout = 2;  //mcu D4 > HX711 dout pin
const int HX711_sck = 14;  //mcu D6 > HX711 sck pin

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_eepromAdress = 0;
unsigned long t = 0;

// Replace with your network credentials
const char* ssid = "Galaxy M31DDB6";
const char* password = "onlyifyouhaveto";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


String readLoadCell() {
  static boolean newDataReady = 0;
  const int serialPrintInterval = 0;  //increase value to slow down serial print activity

  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // get smoothed value from the dataset:
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      float i = LoadCell.getData();
      i = i * 0.00980665;
      Serial.print("Load_cell output val (Newtons): ");
      Serial.println(i);
      newDataReady = 0;
      t = millis();
      return String(i);
    }
  }
  return "";
}


void setup() {

  // Serial port for debugging purposes
  Serial.begin(115200);

  LoadCell.begin();
  //LoadCell.setReverseOutput();
  float calibrationValue;
  calibrationValue = 109.19;

#if defined(ESP8266) || defined(ESP32)
#endif
  //EEPROM.get(calVal_eepromAdress, calibrationValue); // uncomment this if you want to fetch the calibration value from eeprom

  unsigned long stabilizingtime = 5000;  // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true;                  //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1)
      ;
  } else {
    LoadCell.setCalFactor(calibrationValue);  // set calibration value (float)
    Serial.println("LoadCell Startup is complete");
  }

  // Initialize LittleFS
  LittleFS.begin();
  if (!LittleFS.begin()) {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(LittleFS, "/index.html");
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/plain", readLoadCell().c_str());
  });

  // Start server
  server.begin();
  Serial.println("Server initialised");
}

void loop() {
}
