#include "Tadiran.h"
#include <IRremote.hpp>
#include <EspMQTTClient.h>

const uint16_t kIrLed = 12;

EspMQTTClient client(
  "Froot",
  "66666666",
  "192.168.1.150",                                                     // MQTT Broker server ip
  "homeassistant",                                                     // Can be omitted if not needed
  "chiquiingoF9ohsoefoh8ohXoola2quuw2ichahdeiwae7theise7oosheesh7ah",  // Can be omitted if not needed
  "ACControl",                                                         // Client name that uniquely identify your device
  1883                                                                 // The MQTT port, default to 1883. this line can be omitted
);

// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished() {
  // // Subscribe to "mytopic/test" and display received message to Serial
  // client.subscribe("mytopic/test", [](const String& payload) {
  //   Serial.println(payload);
  // });

  // // Subscribe to "mytopic/wildcardtest/#" and display received message to Serial
  // client.subscribe("mytopic/wildcardtest/#", [](const String& topic, const String& payload) {
  //   Serial.println("(From wildcard) topic: " + topic + ", payload: " + payload);
  // });

  // // Publish a message to "mytopic/test"
  // client.publish("mytopic/test", "This is a message");  // You can activate the retain flag by setting the third parameter to true

  // // Execute delayed instructions
  // client.executeDelayed(5 * 1000, []() {
  //   client.publish("mytopic/wildcardtest/test123", "This is a message sent 5 seconds later");
  // });

  client.subscribe("ac/toggle", toggleAC);
}




// the IR emitter object
// IRsend irsend(kIrLed);
int incomingByte = 0;
// A/C Settings
int temperature = 26;
int mode = MODE_auto;     // 0-Auto | 1-Cold | 2-Dry | 3-Fan | 4-Heat
int fanspeed = FAN_auto;  //0-Auto | 1-Low | 2-Medium | 3-High
//A/C Toggles
boolean power = true;

// the Tairan code generator object
Tadiran tadiran(mode, fanspeed, temperature, power == 1 ? 1 : 0);

void setup() {
  Serial.begin(115200);
  // irsend.begin(kIrLed);
  IrSender.begin(kIrLed);
  Serial.println("Commands: +/- Temperature | m - Mode | f - fanspeed | p - Power");

  // Optional functionalities of EspMQTTClient
  client.enableDebuggingMessages();                                           // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater();                                              // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overridded with enableHTTPWebUpdater("user", "password").
  client.enableOTA();                                                         // Enable OTA (Over The Air) updates. Password defaults to MQTTPassword. Port is the default OTA port. Can be overridden with enableOTA("password", port).
  client.enableLastWillMessage("TestClient/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true

  // delay(1000);
  // tadiran.setTemeprature(temperature);
  // tadiran.setMode(mode);
  // tadiran.setFan(fanSpeed);
  // tadiran.setState(power == 1 ? 1 : 0);
  // tadiran.setState(STATE_on);
  Serial.printf("/tadiran 200 - temperature: %i, mode: %i, fanSpeed: %i, power: %i\n",
                temperature, mode, fanspeed, power);
  tadiran.print();
  IrSender.sendRaw(tadiran.codes, sizeof(tadiran.codes) / sizeof(tadiran.codes[0]), 38);
  Serial.println();

  delay(1000);
  tadiran.setState(STATE_off);
  IrSender.sendRaw(tadiran.codes, TADIRAN_BUFFER_SIZE, 38);
}

void loop() {
  client.loop();
  if (Serial.available() > 0) {
    //Serial.write(27);  // ESC command
    //Serial.print("[2J");
    // read the incoming byte:
    incomingByte = Serial.read();
    Serial.println("Incoming Byte: " + incomingByte);

    // say what you got:
    Serial.print("Command: ");
    if (incomingByte == 43) {
      Serial.print("Temperature +");
      if (temperature + 1 <= 30) {
        temperature++;
      }
      tadiran.setTemeprature(temperature);
    }
    if (incomingByte == 45) {
      Serial.print("Temperature -");
      if (temperature - 1 >= 16) {
        temperature--;
      }
      tadiran.setTemeprature(temperature);
    }
    if (incomingByte == 109) {  //mode
      Serial.print("Mode");
      if (mode + 1 <= 4) {
        mode++;
      } else {
        mode = 0;
      }
      tadiran.setMode(mode);
    }
    if (incomingByte == 102) {  //fan
      Serial.print("Fan");
      if (fanspeed + 1 <= 3) {
        fanspeed++;
      } else {
        fanspeed = 0;
      }
      tadiran.setFan(fanspeed);
    }
    if (incomingByte == 112) {  //power
      Serial.print("Power");
      if (power) {
        power = false;
        tadiran.setState(STATE_on);
      } else {
        power = true;
        tadiran.setState(STATE_off);
      }
    }
    Serial.println("");

    tadiran.print();

    Serial.println("");
    Serial.println("Commands: +/- Temperature | m - Mode | f - fanspeed | p - Power");
    IrSender.sendRaw(tadiran.codes, TADIRAN_BUFFER_SIZE, 38);
  }
}

void toggleAC(const String& topic, const String& message) {
  Serial.print("Power");
  if (power) {
    power = false;
    tadiran.setState(STATE_on);
  } else {
    power = true;
    tadiran.setState(STATE_off);
  }

  tadiran.print();
  IrSender.sendRaw(tadiran.codes, TADIRAN_BUFFER_SIZE, 38);
}
