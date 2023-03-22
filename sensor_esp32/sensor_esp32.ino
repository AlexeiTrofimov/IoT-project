#include "DHT.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define DHTPIN 4     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE);

// BLE related settings
#define SERVICE_UUID "91bad492-b950-4226-aa2b-4ede9fa42f59"

BLECharacteristic TemperatureCharacteristics("cba1d466-344c-4be3-ab3f-189f80dd7518", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor TemperatureDescriptor(BLEUUID((uint16_t)0x2902));

BLECharacteristic HumidityCharacteristics("ca73b3ba-39f6-4ab3-91ae-186dc9577d99", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor HumidityDescriptor(BLEUUID((uint16_t)0x2903));

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

float temp;
float hum;

bool deviceConnected = false;

//Setup callbacks onConnect and onDisconnect
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

void setup() {
  Serial.begin(115200);
  
  dht.begin();

  Serial.println("Starting BLE work!");

  BLEDevice::init("Temp/Humidity");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  
  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  pService->addCharacteristic(&TemperatureCharacteristics);
  TemperatureDescriptor.setValue("Temperature in Celsius");
  TemperatureCharacteristics.addDescriptor(&TemperatureDescriptor);
  
  pService->addCharacteristic(&HumidityCharacteristics);
  HumidityDescriptor.setValue("Humidity");
  HumidityCharacteristics.addDescriptor(new BLE2902());

  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);

  pServer->getAdvertising()->start();
  Serial.println("Characteristic defined! Now you can read it in your phone!");

}

void loop() {

  if (deviceConnected) {
    if ((millis() - lastTime) > timerDelay) {
      // Read temperature
      temp = dht.readTemperature();
      
      // Read humidity
      hum = dht.readHumidity();

    if (isnan(hum) || isnan(temp)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }        
  
    //Notify temperature reading from DHT sensor
    static char temperatureCTemp[6];
    dtostrf(temp, 6, 2, temperatureCTemp);
    //Set temperature Characteristic value and notify connected client
    TemperatureCharacteristics.setValue(temperatureCTemp);
    TemperatureCharacteristics.notify();
    Serial.print("Temperature Celsius: ");
    Serial.print(temp);
    Serial.print(" ºC");
    
    //Notify humidity reading from DHT
    static char humidityTemp[6];
    dtostrf(hum, 6, 2, humidityTemp);
    //Set humidity Characteristic value and notify connected client
    HumidityCharacteristics.setValue(humidityTemp);
    HumidityCharacteristics.notify();   
    Serial.print(" - Humidity: ");
    Serial.print(hum);
    Serial.println(" %");
    
    lastTime = millis();
    }
  }

}