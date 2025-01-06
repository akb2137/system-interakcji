//Alicja Babińska 
//Distance-dependent interaction system in location-based games 
//v.0.1.0

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
String deviceName = "hlthy"; //alien for alien; hlthy for healthy; swtch for mode switch
int alienscore = 0; //from 0 to 30 where 0 is heathly and 30 is alien


//global variables regarding scanning
BLEScan *pBLEScan;

//global variables regarding advertising
BLEAdvertising *pAdvertising;



void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));
  
  //scanning setup
  
  BLEDevice::init(deviceName);
  pBLEScan = BLEDevice::getScan();  //create new scan
  pBLEScan->setActiveScan(true);  //passive scan
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
  
  //advertising setup
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(device_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->start();

  //display setup
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }


 //additional setup, display etc.
    if(deviceName == "alien") {
      alienSetup(); 
    } 
    else if(deviceName == "hlthy") {
    hlthySetup(); 
    }

   Serial.println("setup");

}




 void alienSetup() {
    //display things
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
 

void loop() {
  if(alienscore > 30) {alienscore = 30;}
  if(alienscore < 0) {alienscore = 0;}


  int scanTime = random(3,5);
  BLEScanResults *foundDevices = pBLEScan->start(scanTime, true);
  String scannedName = "";
  int scannedRSSI = 0;
  for (int i = 0; i < foundDevices->getCount(); i++) {
    BLEAdvertisedDevice scannedDevice = foundDevices->getDevice(i);
    scannedName = scannedDevice.getName();
    scannedRSSI = scannedDevice.getRSSI();
    Serial.printf("name %s rssi: %d \n", scannedName.c_str(), scannedRSSI);
  }
  pBLEScan->clearResults();

  int delayTime = (6-scanTime)*1000;
  delay(delayTime);
  Serial.printf("delayed by %d, scantime :%d", delayTime, scanTime);
  

}
