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

//library for buzzer
#include "pitches.h"


//defining display settings
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


const int buzzerPin = 25;


//global variables 
//CONFIGURE STARTING PARAMETERS HERE!!!
//-----------------------------------
String deviceName = "player"; //player (weather alien or healthy) or chapel for mode switching station
bool isAlien = false; //true if is alien
int alienscore = 0; //from 0 to 15 where 0 is heathly and 15 is alien
//-------------------------------------- values dependent on conditions
int minAlienRSSI = -80;
int minChapelRSSI = -75;
//-----------------------------------
int scansWithoutAlien = 0; //we don't want to delete alienscore after one scan not showing any aliens - it could be due to lost data in transfer etc.
int scansWithoutChapel = 0; 


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
  }
  display.clearDisplay();

  pinMode(buzzerPin, OUTPUT);
  tone(buzzerPin, NOTE_B3, 1000/4);
  noTone(buzzerPin);

  BLEDevice::init(deviceName);
  BLEinit();


 //additional setup, display etc.
    if((isAlien) && (deviceName == "player")) {
      Serial.println("\n starting advertising");
      Serial.printf("Free heap: %d bytes\n", esp_get_free_heap_size());
      //pAdvertising->start();
      Serial.println("\n advertising");
      alienSetup(); 
      Serial.printf("Free heap: %d bytes\n", esp_get_free_heap_size());
    } 
    else if((!isAlien) && (deviceName == "player")) {
      healthySetup(); 
    }
    else if(deviceName == "chapel") {
      pAdvertising->start();
      chapelSetup();
    }
   Serial.println("setup");

}


void BLEinit() { //this function will be called whenever device name changes
  
  Serial.printf("Reinitializing BLE with name: %s\n", deviceName.c_str());
  //scanning setup

if(deviceName == "player") {
  pBLEScan = BLEDevice::getScan();  //create new scan
  pBLEScan->setActiveScan(true);  //false for passive scan
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}
  
  //advertising setup
  Serial.printf("got past init");
  pAdvertising = BLEDevice::getAdvertising();
  BLEAdvertisementData advData;
  advData.setName(deviceName);
  pAdvertising->setAdvertisementData(advData);
  pAdvertising->setScanResponse(true);

}

void alienSetup() {
  
    delay(30000); //30 second until player "feels" first symptoms. can be changed to millis, but here it fits to be a delay (aka stalling whole program)

    //display things
    for(int i = 30; i > 0 ; i--) {
    display.clearDisplay();
    Serial.println("starting display setup 2");
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,1);
    display.printf("! ! ! ! ! ! ! ! ! ! !");
    display.setCursor(0,20);
    display.printf("you start to feel \n a little... weird");
    display.setTextSize(1);
    display.setCursor(1,56);
    display.printf("%d", i);
    display.display();
    delay(1000);
    }

    delay(1000);
    
    //TODO: odliczanie do zaczęcia zarażania 
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(BLACK, WHITE);
    display.setCursor(18,20);
    display.printf(" ALIEN ");
    display.display();
 }

void healthySetup() {
    //display things
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,20);
    display.printf("you feel OK");
    display.display();
  }


void chapelSetup() {
    //display things
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.printf("+ chapel +");
    display.setTextSize(1);
    display.setCursor(0,30);
    display.printf("of.. healing?");
    display.display();
  }
 

void turnToAlien() {
    alienscore = 15;
    isAlien = true;
    delay(500);
    alienSetup(); 
    pAdvertising->start();
}

void turnToHealthy() {
    pAdvertising->stop();
    alienscore = 0;
    isAlien = false;
    delay(500);
    healthySetup();
}



void alienLoop() {
  
  int scanTime = random(2,5);
  BLEScanResults *foundDevices = pBLEScan->start(scanTime, true);
  bool thereIsChapelNearby = false;
  String scannedName = "";
  int scannedRSSI = 0;
  Serial.printf("\n scanning more");
  Serial.printf("\n count: %d", foundDevices->getCount());
  for (int i = 0; i < foundDevices->getCount(); i++) {
    BLEAdvertisedDevice scannedDevice = foundDevices->getDevice(i);
    scannedName = scannedDevice.getName();
    scannedRSSI = scannedDevice.getRSSI();
    Serial.printf("name %s rssi: %d \n", scannedName.c_str(), scannedRSSI);
    if((scannedName == "chapel") && (scannedRSSI >= minChapelRSSI)) {
    thereIsChapelNearby = true;
    scansWithoutChapel = 0;
    }
  }

  if (thereIsChapelNearby) {
    
    alienscore--;
    display.setCursor(0,1);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.printf("+ + + + + + + + + + +");
    display.display();
  }
    else if (!thereIsChapelNearby) {
    scansWithoutChapel++;
  }
  
  if (scansWithoutChapel >= 2) {
    alienscore = 15;
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(BLACK, WHITE);
    display.setCursor(18,20);
    display.printf(" ALIEN ");
    display.display();
      
  }
  
  pBLEScan->clearResults();

  int delayTime = (6-scanTime)*1000;
  delay(delayTime);
  Serial.printf("\n delayed by %d, scantime :%d, alienscore: %d", delayTime, scanTime, alienscore);
  
}

void healthyLoop() {
  
  int scanTime = random(3,6);
  BLEScanResults *foundDevices = pBLEScan->start(scanTime, true);
  bool thereIsAlienNearby = false; //setting this boolean value rather than changing alienscore on scan so that many aliens in range don't cause instant change. Note that to turn to alien there must be at least one alien for a minute and a half nearby, no the same alien. 
  bool thereIsChapelNearby = false;
  String scannedName = "";
  int scannedRSSI = 0;
  Serial.printf("\n count: %d", foundDevices->getCount());
  for (int i = 0; i < foundDevices->getCount(); i++) {
    BLEAdvertisedDevice scannedDevice = foundDevices->getDevice(i);
    scannedName = scannedDevice.getName();
    scannedRSSI = scannedDevice.getRSSI();
    Serial.printf("name %s rssi: %d \n", scannedName.c_str(), scannedRSSI);
    if((scannedName == "player") && (scannedRSSI >= minAlienRSSI)) {
      Serial.printf("alien found");
      thereIsAlienNearby = true;
      scansWithoutAlien = 0;
    }
    else if((scannedName == "chapel") && (scannedRSSI >= minChapelRSSI)) {
      thereIsChapelNearby = true;
      scansWithoutChapel = 0;
    }
  }

  
  if (thereIsAlienNearby) {
    Serial.printf("\n bad alien");
     alienscore++;
  }
    else if (!thereIsAlienNearby) {
    scansWithoutAlien++;
  }
  if (thereIsChapelNearby) {
    alienscore = alienscore + 5;
    tone(buzzerPin, NOTE_C1, 1000/4);
  }
    else if (!thereIsChapelNearby) {
    scansWithoutChapel++;
  }

  
  if ((scansWithoutChapel >= 2) && (scansWithoutAlien >= 2)) {
    alienscore = 0;  
  }
  
  pBLEScan->clearResults();

  int delayTime = (6-scanTime)*1000;
 delay(delayTime);
 Serial.printf("\n delayed by %d, scantime :%d, alienscore: %d, scans without alien: %d", delayTime, scanTime, alienscore, scansWithoutAlien);
  
}

 

void loop() {
  Serial.printf("\n begin loop\n name: %s", deviceName);
  noTone(buzzerPin);

  if(alienscore > 15) {alienscore = 15;}
  if(alienscore < 0) {alienscore = 0;} 

  if(deviceName == "player") {
    if((alienscore == 15) && (!isAlien)) {
      Serial.printf(deviceName.c_str());
      turnToAlien();
    }
    if((alienscore == 0) && (isAlien)) {
      Serial.printf(deviceName.c_str());
      turnToHealthy();
    }
    if(isAlien) {
      alienLoop();
    }
    else {
      healthyLoop();
    }
    
  }

  else if ((deviceName == "chapel")) {
    delay(200);
  }

  

}
