//Alicja Babińska 
//Distance-dependent interaction system in location-based games 
//v.0.1.1

//wersja kodu w której po zmianie trybu advertisowana nazwa zmienia się czasami XD

//libraries for BLE
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>



//libraries for display
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


//defining display settings
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define device_UUID "00001234-0000-1000-8000-00805f9b34fb"


//global variables 
//CONFIGURE STARTING PARAMETERS HERE!!!
//-----------------------------------
String deviceName = "alien"; //alien for alien; hlthy for healthy; swtch for mode switch
int alienscore = 15; //from 0 to 15 where 0 is heathly and 15 is alien
//-----------------------------------

//global variables regarding scanning
BLEScan *pBLEScan;

//global variables regarding advertising
BLEAdvertising *pAdvertising;



void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));
  
  //display setup
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  BLEDevice::init(deviceName);
  BLEinit();


 //additional setup, display etc.
    if(deviceName == "alien") {
      alienSetup(); 
    } 
    else if(deviceName == "hlthy") {
      hlthySetup(); 
    }

   Serial.println("setup");

}


void BLEinit() { //this function will be called whenever device name changes
  
  Serial.printf("Reinitializing BLE with name: %s\n", deviceName.c_str());
  //scanning setup

  pBLEScan = BLEDevice::getScan();  //create new scan
  pBLEScan->setActiveScan(true);  //passive scan
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
  
  //advertising setup
  Serial.printf("got past init");
  pAdvertising = BLEDevice::getAdvertising();
  BLEAdvertisementData advData;
  advData.setName(deviceName);
  pAdvertising->setAdvertisementData(advData);
  pAdvertising->addServiceUUID(device_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->start();
}

void alienSetup() {
    //display things
    
    //TODO: odliczanie do zaczęcia zarażania 
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,20);
    display.printf("Alien!");
    display.display();
 }

void hlthySetup() {
    //display things
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,20);
    display.printf("Not Alien!");
    display.display();
  }

  //TODO: modeswitch setup 

 

void loop() {
  Serial.printf("\n begin loop");
  if((alienscore >= 15) && (deviceName != "alien")) {
    Serial.printf(deviceName.c_str());
    alienscore = 15;
    deviceName = "alien";
    pAdvertising->stop();
    delay(500);
    BLEinit();
    alienSetup();
    }
  if((alienscore <= 0) && (deviceName != "hlthy")) {
    Serial.printf(deviceName.c_str());
    alienscore = 0;
    deviceName = "hlthy";
    pAdvertising->stop();
    delay(500);
    BLEinit();
    hlthySetup();
    }

  delay(100);
  Serial.printf("\n scanning");
  int scanTime = random(3,6);
  BLEScanResults *foundDevices = pBLEScan->start(scanTime, true);
  String scannedName = "";
  int scannedRSSI = 0;
  Serial.printf("\n scanning more");
  Serial.printf("\n count: %d", foundDevices->getCount());
  for (int i = 0; i < foundDevices->getCount(); i++) {
    BLEAdvertisedDevice scannedDevice = foundDevices->getDevice(i);
    scannedName = scannedDevice.getName();
    scannedRSSI = scannedDevice.getRSSI();
    Serial.printf("name %s rssi: %d \n", scannedName.c_str(), scannedRSSI);
    if((scannedName == "alien") && (scannedRSSI >= -80)) {
      alienscore = alienscore + 10;
    }
  }
  pBLEScan->clearResults();

  int delayTime = (6-scanTime)*1000;
  delay(delayTime);
  Serial.printf("\n delayed by %d, scantime :%d, alienscore: %d", delayTime, scanTime, alienscore);
  

}
