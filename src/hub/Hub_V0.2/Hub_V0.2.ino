/***********************************
   Author: Peter Dorich
   Based off code from OSC by Kenny Noble
   Back-end for send/ recieve data between hub/ valve
 ************************************/
#include <SPI.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>
#include <OSCBundle.h>

/* for feather32u4
  #define RFM95_CS 8
  #define RFM95_RST 4
  #define RFM95_INT 7
*/
/* for M0 */
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3
#define LED 13


#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include "Ethernet2.h"    //Required for ethernet, Ethernet.h will not work

#include <Dns.h>
#include <Dhcp.h>

#define HUB_ADDRESS 1
#define RELAY_ADDRESS 2

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

//Size of message for LoRa
#define MSG_SIZE 121

//IDString constructor
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x) //to concatenate a predefined number to a string literal, use STR(x)

#define FAMILY "/LOOM"
#define DEVICE "/Hub/"
#define INSTANCE_NUM 0  // Unique instance number for this device, useful when using more than one of the same device type in same space

#define IDString FAMILY DEVICE STR(INSTANCE_NUM) // C interprets subsequent string literals as concatenation: "/Loom" "/Ishield" "0" becomes "/Loom/Ishield0"


// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

RHReliableDatagram manager(rf95, HUB_ADDRESS);
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

/********************Ethernet Client Setup******************************/
byte mac[] = {0x98, 0x76, 0xB6, 0x10, 0x61, 0xD6};

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "OPEnS"
#define AIO_KEY         "c3b8ceca3231410ab47418540810c1fe"

EthernetClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// You don't need to change anything below this line!
#define halt(s) { Serial.println(F( s )); while(1);  }

Adafruit_MQTT_Publish Elec_Cond = Adafruit_MQTT_Publish(&mqtt,  AIO_USERNAME "/feeds/soil-data.elec-cond");
Adafruit_MQTT_Publish Temperature = Adafruit_MQTT_Publish(&mqtt,  AIO_USERNAME "/feeds/soil-data.temp");
Adafruit_MQTT_Publish VWC = Adafruit_MQTT_Publish(&mqtt,  AIO_USERNAME "/feeds/soil-data.vwc");
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt,  AIO_USERNAME "/feeds/soil-data.on-off");



void setup() {
  Serial.begin(9600);
  while (!Serial);

  //manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  if (!manager.init())
    Serial.println("init failed");

  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }

  rf95.setTxPower(23, false);

  Ethernet.begin(mac);
  //delay(1000);

  mqtt.subscribe(&onoffbutton);

  MQTT_connect();
}

uint32_t x = 0;

struct soil_data
{
  float VWC;
  float TEMP;
  uint32_t ELEC_COND;

};

soil_data s_dat;

OSCBundle inst_bndl;

void loop() {
  // put your main c here, to run repeatedly:l
  // soil_data f_dat;

  //MQTT_connect();

  // delay(5 * 1000);
  //Adafruit_MQTT_Subscribe *subscription;
  /* while ((subscription = mqtt.readSubscription(1000))) {
     if (subscription == &onoffbutton) {
       Serial.print(F("Got: "));
       Serial.println((char *)onoffbutton.lastread);
     }
    if (strcmp((char *)onoffbutton.lastread, "ON") == 0) {
         digitalWrite(LED, HIGH);
       }
    if (strcmp((char *)onoffbutton.lastread, "OFF") == 0) {
         digitalWrite(LED, LOW);
       }

    }
  */
  /*
    s_dat.ELEC_COND = 25;
    s_dat.TEMP = 34.5;
    s_dat.VWC = 10.21;
  */

  // Elec_Cond.publish(s_dat.ELEC_COND, DEC);
  /*  if (! Elec_Cond.publish(s_dat.ELEC_COND)) {
       Serial.println(F("Failed"));
       } else {
          Serial.println(F("Got Elec_Cond"));
       }
    if (! Temperature.publish((char *) String(s_dat.TEMP).c_str())) {
       Serial.println(F("Failed"));
       } else {
          Serial.println(F("Got Temp"));
       }
    if (! VWC.publish((char *) String(s_dat.VWC).c_str())) {
       Serial.println(F("Failed"));
       } else {
          Serial.println(F("Got VWC"));
       }
  */
  //delay(7000);
  // Adafruit_MQTT_Subscribe *subscription;
  Adafruit_MQTT_Subscribe *subscription;

  unsigned long now = millis();
  int x = 0;
  while (!manager.available() && (millis() - now < 10000)) {
    x++;
    while ((subscription = mqtt.readSubscription(250))) {
      if (subscription == &onoffbutton) {
        Serial.print(F("Got: "));
        Serial.println((char *)onoffbutton.lastread);
      }
      if (strcmp((char *)onoffbutton.lastread, "ON") == 0) {
        digitalWrite(LED, HIGH);
      }
      if (strcmp((char *)onoffbutton.lastread, "OFF") == 0) {
        digitalWrite(LED, LOW);
      }

    }
  }
  if (manager.available()) {
    uint8_t len = sizeof(buf);
    uint8_t from;
    memset(buf, '\0', RH_RF95_MAX_MESSAGE_LEN);
    if (manager.recvfromAck(buf, &len, &from)) {
      OSCBundle bndl;
      get_OSC_bundle((char*)buf, &bndl);
      Serial.println((char*)buf);
      bndl.send(Serial);
      Serial.println("");


      //----------------------------------------
      //------ Pack and send instructions ------
      //----------------------------------------
      char inst_mess[MSG_SIZE];
      memset(inst_mess, '\0', MSG_SIZE);

      float inst_VWC = 20.1;
      int32_t inst_timer = 5000;

      inst_bndl.empty();

      inst_bndl.add(IDString "/VWC_Inst").add((float)inst_VWC);
      inst_bndl.add(IDString "/Time_Inst").add((int32_t) inst_timer);
      inst_bndl.add(IDString "/Mode_Inst").add((int32_t) 0);

      get_OSC_string(&inst_bndl, inst_mess);

      if (manager.sendtoWait((uint8_t*)inst_mess, strlen(inst_mess), RELAY_ADDRESS)) {
        Serial.println("Instructions sent.");
      } else {
        Serial.println("Instruction sending failed -- TODO/Resend?");
      }
      //----------------------------------------
      //------ End of instruction passing ------
      //----------------------------------------

      //Publish Info to Adafruit.io
      if (! Elec_Cond.publish(s_dat.ELEC_COND)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("Got Elec_Cond"));
      }
      if (! Temperature.publish((char *) String(s_dat.TEMP).c_str())) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("Got Temp"));
      }
      if (! VWC.publish((char *) String(s_dat.VWC).c_str())) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("Got VWC"));
      }

    }
  }
}

void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}
