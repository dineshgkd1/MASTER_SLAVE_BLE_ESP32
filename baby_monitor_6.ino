#include "BLEDevice.h"
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

static BLEUUID serviceUUID("00001809-0000-1000-8000-00805F9B34FB");
static BLEUUID    charUUID("00002a1c-0000-1000-8000-00805F9B34FB");

static BLEUUID serviceUUID1("00001822-0000-1000-8000-00805F9B34FB");
static BLEUUID    charUUID1("00002A62-0000-1000-8000-00805F9B34FB");
static BLEUUID    charUUID2("00002A92-0000-1000-8000-00805F9B34FB");

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
BLECharacteristic* bCharacteristic = NULL;
BLECharacteristic* cCharacteristic = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

#define SERVICE_UUID        "6ee164c5-c9de-4a40-a939-d9c6864928ec"
#define CHARACTERISTIC_UUID "711cfcd9-0794-4874-899f-56c0a5ea63d8"

#define SERVICE_UUID1        "6ee164c6-c9de-4a40-a939-d9c6864928ec"
#define CHARACTERISTIC_UUID1 "711cfcd1-0794-4874-899f-56c0a5ea63d8"
#define CHARACTERISTIC_UUID2 "711cfcd1-0794-4874-899f-56c0a5ea63d9"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising();
    };
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLERemoteCharacteristic* bRemoteCharacteristic;
static BLERemoteCharacteristic* cRemoteCharacteristic;

static BLEAdvertisedDevice* myDevice;

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  pCharacteristic->notify();
  Serial.println((char*)pData);
  pCharacteristic->setValue((char*)pData);
}

static void notifyCallback1(
  BLERemoteCharacteristic* bBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  bCharacteristic->notify();
  Serial.println((char*)pData);
  bCharacteristic->setValue((char*)pData);
}

static void notifyCallback2(
  BLERemoteCharacteristic* cBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  cCharacteristic->notify();
  Serial.println((char*)pData);
  cCharacteristic->setValue((char*)pData);
}

class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pclient) {
    }

    void onDisconnect(BLEClient* pclient) {
      connected = false;
      //      Serial.println("onDisconnect");
    }
};

bool connectToServer() {
  BLEClient*  pClient  = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallback());
  pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)

  Serial.println(" - Connected to server");
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);

  if (pRemoteService == nullptr) {
    //Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }

  Serial.println(" - Found our service");
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr) {
    pClient->disconnect();
    return false;
  }

  BLERemoteService* bRemoteService = pClient->getService(serviceUUID1);

  Serial.println(" - Found our service");
  bRemoteCharacteristic = bRemoteService->getCharacteristic(charUUID1);
  if (bRemoteCharacteristic == nullptr) {
    pClient->disconnect();
    return false;
  }

  cRemoteCharacteristic = bRemoteService->getCharacteristic(charUUID2);
  if (bRemoteCharacteristic == nullptr) {
    pClient->disconnect();
    return false;
  }

  Serial.println(" - Found our characteristic");
  if (pRemoteCharacteristic->canRead()) {
    std::string value = pRemoteCharacteristic->readValue();
    Serial.println("The characteristic value was: 1");
  }

  Serial.println(" - Found our characteristic");
  if (bRemoteCharacteristic->canRead()) {
    std::string value = bRemoteCharacteristic->readValue();
    Serial.println("The characteristic value was: 2");
  }

  if (cRemoteCharacteristic->canRead()) {
    std::string value = bRemoteCharacteristic->readValue();
    Serial.println("The characteristic value was: 2");
  }

  if (pRemoteCharacteristic->canNotify())
    pRemoteCharacteristic->registerForNotify(notifyCallback);

  if (bRemoteCharacteristic->canNotify())
    bRemoteCharacteristic->registerForNotify(notifyCallback1);

  if (cRemoteCharacteristic->canNotify())
    cRemoteCharacteristic->registerForNotify(notifyCallback2);
  
  connected = true;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.println("BLE Advertised Device found: ");
      if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

        BLEDevice::getScan()->stop();
        myDevice = new BLEAdvertisedDevice(advertisedDevice);
        doConnect = true;
        doScan = true;

      } // Found our server
    } // onResult
}; // MyAdvertisedDeviceCallbacks

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("BACHU");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLEService *bService = pServer->createService(SERVICE_UUID1);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  bCharacteristic = bService->createCharacteristic(
                      CHARACTERISTIC_UUID1,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  cCharacteristic = bService->createCharacteristic(
                      CHARACTERISTIC_UUID2,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  pCharacteristic->addDescriptor(new BLE2902());
  bCharacteristic->addDescriptor(new BLE2902());
  cCharacteristic->addDescriptor(new BLE2902());

  pService->start();
  bService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  BLEAdvertising *bAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter

  bAdvertising->addServiceUUID(SERVICE_UUID1);
  bAdvertising->setScanResponse(false);
  bAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter

  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}
void loop() {
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }
  if (connected) {
    BLEDevice::getScan()->start(0);  // this is just eample to start scan after disconnect, most likely there is better way to do it in arduino
  }

  delay(1000); // Delay a second between loops.

  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
}
