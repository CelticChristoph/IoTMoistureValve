#include <SPI.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>
#include <OSCBundle.h>
 
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

#define HUB_ADDRESS 1
#define RELAY_ADDRESS 2 

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0
 
// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

RHReliableDatagram manager(rf95, HUB_ADDRESS);
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void setup() {
  Serial.begin(9600);
  if (!manager.init())
    Serial.println("init failed");

  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  
  rf95.setTxPower(23, false);

}

OSCBundle bndl;

void loop() {
  //while (!manager.available());
  if (manager.available()) {
    uint8_t len = sizeof(buf);
    uint8_t from;
    memset(buf, '\0', RH_RF95_MAX_MESSAGE_LEN);
    if (manager.recvfromAck(buf, &len, &from)) {
      get_OSC_bundle((char*)buf, &bndl);
      Serial.print("Received message: ");
      Serial.println((char*)buf);
      bndl.send(Serial);
      Serial.println("");
    }
  }
}

