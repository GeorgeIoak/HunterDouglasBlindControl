#include <IRremoteESP8266.h>
//#include <IRremoteInt.h>

//#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "fauxmoESP.h"

#include "credentials.h" //WiFi SSID and password 

#define SERIAL_BAUDRATE                 115200
#define LED                             16  // LED on Board
#define RELAY_PIN                        5  // 
#define OPen2Pin                        12 // Open Blinds pin
#define Close2Pin                       14 // Close Blinds pin
#define khz                             40 // 40kHz carrier frequency for the Hunter Douglas

IRsend irsend(4); //

unsigned int OPen2Signal[] = {2800, 2800, 1050, 2800, 1050, 1050, 1050, 1050, 1050,
                              2800, 2800, 2800, 1050, 2800, 1050, 1050, 1050, 1050, 1050,
                              2800, 2800, 2800, 1050, 2800, 1050, 1050, 1050, 1050, 1050,
                              2800, 2800, 2800, 1050, 2800, 1050, 1050, 1050, 1050, 1050,
                              2800, 2800, 2800, 1050, 2800, 1050, 1050, 1050, 1050, 1050,
                              2800, 2800, 2800, 1050, 2800, 1050, 1050, 1050, 1050, 1050,
                              2800, 2800, 2800, 1050, 2800, 1050, 1050, 1050, 1050, 1050,
                              2800, 2800, 2800, 1050, 2800, 1050, 1050, 1050, 1050, 1050, 1975
                             }; //Open2

unsigned int Close2Signal[] = {2800, 2800, 1050, 2800, 1050, 1050, 1050, 2800, 1050,
                               1050, 2800, 2800, 1050, 2800, 1050, 1050, 1050, 2800, 1050,
                               1050, 2800, 2800, 1050, 2800, 1050, 1050, 1050, 2800, 1050,
                               1050, 2800, 2800, 1050, 2800, 1050, 1050, 1050, 2800, 1050,
                               1050, 2800, 2800, 1050, 2800, 1050, 1050, 1050, 2800, 1050,
                               1050, 2800, 2800, 1050, 2800, 1050, 1050, 1050, 2800, 1050,
                               1050, 2800, 2800, 1050, 2800, 1050, 1050, 1050, 2800, 1050,
                               1050, 2800, 2800, 1050, 2800, 1050, 1050, 1050, 2800, 1050, 1975
                              }; //Close the lower blinds


// variables will change:
int butOpen2State = 0;         // variable for reading the pushbutton status
int butClose2State = 0;        // variable for reading the pushbutton status

fauxmoESP fauxmo;

// -----------------------------------------------------------------------------
// Wifi
// -----------------------------------------------------------------------------

void wifiSetup() {

  // Set WIFI module to STA mode
  WiFi.mode(WIFI_STA);

  // Connect
  Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Wait
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  // Connected!
  Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
}

void callback(unsigned char device_id, const char * device_name, bool state) {
  Serial.print("Device ID "); Serial.print(device_id);
  Serial.print("Device "); Serial.print(device_name);
  Serial.print(" state: ");
  if (state) {
    Serial.println("ON");
  } else {
    Serial.println("OFF");
  }

  if ( (strcmp(device_name, "relay") == 0) ) {
    // adjust the relay immediately!
    if (state) {
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("I'm turning the RELAY ON");
    } else {
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("I'm turning the RELAY OFF");
    }
  }

  if ( (strcmp(device_name, "pixels") == 0) ) {
    if (state) {
      digitalWrite(LED, LOW);
      Serial.println("I'm turning the LED ON");
    } else {
      digitalWrite(LED, HIGH);
      Serial.println("I'm turning the LED OFF");
    }
  }

  if ( (strcmp(device_name, "blinds") == 0) ) {
    if (state) {
      irsend.sendRaw(OPen2Signal, sizeof(OPen2Signal) / sizeof(OPen2Signal[0]), khz);
      Serial.println("I'm opening the blinds");
    } else {
      irsend.sendRaw(Close2Signal, sizeof(Close2Signal) / sizeof(Close2Signal[0]), khz);
      Serial.println("I'm closing the blinds");
    }
  }
}

void setup() {
  // Init serial port and clean garbage
  Serial.begin(SERIAL_BAUDRATE);
  Serial.println("FauxMo demo sketch");
  Serial.println("After connection, ask Alexa/Echo to 'turn <devicename> on' or 'off'");

  // Wifi
  wifiSetup();

  // Buttons
  pinMode(OPen2Pin, INPUT);
  pinMode(Close2Pin, INPUT);

  // LED
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // IR Pin
  //pinMode(4, OUTPUT);
  //digitalWrite(4, LOW);
  irsend.begin();

  // Fauxmo
  fauxmo.addDevice("relay");
  fauxmo.addDevice("pixels");
  fauxmo.addDevice("blinds");
  //fauxmo.addDevice("pixels");
  fauxmo.onMessage(callback);

}

void loop() {

  // Since fauxmoESP 2.0 the library uses the "compatibility" mode by
  // default, this means that it uses WiFiUdp class instead of AsyncUDP.
  // The later requires the Arduino Core for ESP8266 staging version
  // whilst the former works fine with current stable 2.3.0 version.
  // But, since it's not "async" anymore we have to manually poll for UDP
  // packets
  fauxmo.handle();

  // read the state of the pushbutton value:
  butOpen2State = digitalRead(OPen2Pin);
  butClose2State = digitalRead(Close2Pin);
  //Serial.println(butOpen2State);

  if (butOpen2State == HIGH) {
    // send IR code:
    irsend.sendRaw(OPen2Signal, sizeof(OPen2Signal) / sizeof(OPen2Signal[0]), khz);
    Serial.println("Open2 Button Pushed");
    digitalWrite(LED, HIGH);
  }

  if (butClose2State == HIGH) {
    // send IR code:
    irsend.sendRaw(Close2Signal, sizeof(Close2Signal) / sizeof(Close2Signal[0]), khz);
    Serial.println("Close2 Button Pushed");
    digitalWrite(LED, LOW);
  }
}
