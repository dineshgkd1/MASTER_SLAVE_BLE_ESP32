#include "BLEDevice.h"
//#include "BLEScan.h"
#include <Arduino.h>

static BLEUUID serviceUUID("180a");
static BLEUUID    charUUID("2a23");

static BLEUUID serviceUUID1("fff0");
static BLEUUID    charUUID1("fff4");

static BLEUUID serviceUUID2("CDEACB80-5235-4C07-8846-93A37EE6B86D");
static BLEUUID    charUUID2("CDEACB81-5235-4C07-8846-93A37EE6B86D");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLERemoteCharacteristic* qRemoteCharacteristic;

static BLEAdvertisedDevice* myDevice;
static BLEAdvertisedDevice* myDevice1;


static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  Serial.println(sizeof(length));
  if (length == 2) {
    Serial.print("Presure: ");
    //    Serial.println(int(pData[0]));
    Serial.println(int(pData[1]));
  }

  if (length == 18) {
    int sys = (int(pData[1]) * 256) + int(pData[2]);
    int dia = (int(pData[3]) * 256) + int(pData[4]);
    Serial.print("SYS: ");
    Serial.println(sys);
    Serial.print("DIA: ");
    Serial.println(dia);
  }


}

static void notifyCallback1(
  BLERemoteCharacteristic* qBLERemoteCharacteristic,
  uint8_t* qData,
  size_t length,
  bool isNotify) {
  if (length == 4) {
    if (int(qData[1]) != 255 && int(qData[2]) != 127) {
      Serial.print("Pulse: ");
      Serial.println(int(qData[1]));
      Serial.print("SPO2: ");
      Serial.println(int(qData[2]));
    }
  }
}
class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pclient) {
      Serial.println("Connect to Device");
    }

    void onDisconnect(BLEClient* pclient) {
      connected = false;
      Serial.println("onDisconnect");
    }
};

class MyClientCallback1 : public BLEClientCallbacks {
    void onConnect(BLEClient* qclient) {
      Serial.println("Connect to Device");
    }

    void onDisconnect(BLEClient* qclient) {
      connected = false;
      Serial.println("onDisconnect");
    }
};

bool connectToServer() {
  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());
  Serial.println(myDevice1->getAddress().toString().c_str());

  BLEClient*  pClient  = BLEDevice::createClient();
  Serial.println(" - Created client");
  pClient->setClientCallbacks(new MyClientCallback());
  pClient->connect(myDevice);
  Serial.println(" - Connected to server");

  BLEClient*  qClient  = BLEDevice::createClient();
  qClient->setClientCallbacks(new MyClientCallback1());
  qClient->connect(myDevice1);

  BLERemoteService* pRemoteService = pClient->getService(serviceUUID1);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service 1");

  BLERemoteService* qRemoteService = qClient->getService(serviceUUID2);
  if (qRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    qClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service 2");

  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID1);
  if (pRemoteCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our characteristic 1");

  qRemoteCharacteristic = qRemoteService->getCharacteristic(charUUID2);
  if (qRemoteCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID.toString().c_str());
    qClient->disconnect();
    return false;
  }
  Serial.println(" - Found our characteristic 2");

  if (pRemoteCharacteristic->canNotify())
    pRemoteCharacteristic->registerForNotify(notifyCallback);
  if (qRemoteCharacteristic->canNotify())
    qRemoteCharacteristic->registerForNotify(notifyCallback1);

  connected = true;
  return true;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.print("BLE Advertised Device found: ");
      Serial.println(advertisedDevice.toString().c_str());

      // We have found a device, let us now see if it contains the service we are looking for.
      if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

        BLEDevice::getScan()->stop();
        myDevice = new BLEAdvertisedDevice(advertisedDevice);
        doConnect = true;
        doScan = true;
      }
    }
};

class MyAdvertisedDeviceCallbacks1: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.print("BLE Advertised Device found: ");
      Serial.println(advertisedDevice.toString().c_str());

      // We have found a device, let us now see if it contains the service we are looking for.
      if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID2)) {

        BLEDevice::getScan()->stop();
        myDevice1 = new BLEAdvertisedDevice(advertisedDevice);
        doConnect = true;
        doScan = true;
      }
    }
};


void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("BPM_01");
  BLEDevice::init("Medical");

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);

  BLEScan* qBLEScan = BLEDevice::getScan();
  qBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks1());
  qBLEScan->setInterval(1349);
  qBLEScan->setWindow(449);
  qBLEScan->setActiveScan(true);
  qBLEScan->start(5, false);
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
    String newValue = "Time since boot: " + String(millis() / 1000);
    Serial.println("Setting new characteristic value to \"" + newValue + "\"");
    pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
  } else if (doScan) {
    BLEDevice::getScan()->start(0);
  }
  delay(1000);
}
