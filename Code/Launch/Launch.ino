#include <SPI.h>
#include <SD.h>
#include <Arduino.h>
#include <HX711_ADC.h>

//pins:
const int HX711_dout = 2;  //mcu > HX711 dout pin
const int HX711_sck = 14;  //mcu > HX711 sck pin

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_eepromAdress = 0;
unsigned long t = 0;

// Replace with your network credentials
const char* ssid = "Xiaomi 11 Lite NE";
const char* password = "password01";

File myFile;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);

  LoadCell.begin();
  //LoadCell.setReverseOutput();
  float calibrationValue;
  calibrationValue = 109.19;

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

  Serial.print("Initializing SD card...");

  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
}


void loop() {
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
      myFile = SD.open("test_data.txt", FILE_WRITE);

      // if the file opened okay, write to it:
      if (myFile) {
        myFile.println(t, i);
      } else {
        // if the file didn't open, print an error:
        Serial.println("error opening test.txt");
      }
      myFile.close();
    }
  }
}
