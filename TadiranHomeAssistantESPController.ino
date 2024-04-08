#include "Tadiran.h"
// #include <M5Atom.h>
// #include <FastLED.h>
#include <LiteLED.h>
#include <IRremote.hpp>
#include <EspMQTTClient.h>

const uint16_t kIrLed = 12;

// Choose the LED type from the list below.
// #define LED_TYPE LED_STRIP_WS2812
#define LED_TYPE LED_STRIP_SK6812
// #define LED_TYPE LED_STRIP_APA106
// #define LED_TYPE LED_STRIP_SM16703

#define LED_TYPE_IS_RGBW 1
#define LED_GPIO 27
#define LED_BRIGHT 100

static const crgb_t L_RED = 0xff0000;
static const crgb_t L_GREEN = 0x00ff00;
static const crgb_t L_BLUE = 0x0000ff;
static const crgb_t L_WHITE = 0xe0e0e0;

LiteLED myLED( LED_TYPE, LED_TYPE_IS_RGBW );

EspMQTTClient client(
  "Froot",
  "66666666",
  "192.168.1.150",                                                     // MQTT Broker server ip
  "homeassistant",                                                     // Can be omitted if not needed
  "chiquiingoF9ohsoefoh8ohXoola2quuw2ichahdeiwae7theise7oosheesh7ah",  // Can be omitted if not needed
  "ACControl",                                                         // Client name that uniquely identify your device
  1883                                                                 // The MQTT port, default to 1883. this line can be omitted
);

bool light = true;

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

  client.subscribe("ron/ac/power/set", toggleAC);
  client.subscribe("ron/ac/light/set", toggleACLight);
  client.subscribe("ron/ac/mode/set", setMode);
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
  myLED.begin( LED_GPIO, 1 );
  myLED.brightness( LED_BRIGHT );
  myLED.setPixel( 0, L_BLUE, 1 );


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
  Serial.println("Power");
  Serial.println(message);
  if (power) {
    tadiran.setState(STATE_on);
    myLED.setPixel( 0, L_GREEN, 1 );
  } else {
    tadiran.setState(STATE_off);
    myLED.setPixel( 0, L_RED, 1 );
  }

  tadiran.print();
  IrSender.sendRaw(tadiran.codes, TADIRAN_BUFFER_SIZE, 38);
}

void setMode(const String& topic, const String& message) {
  Serial.println();
  Serial.println("Setting Mode:");
  Serial.println(message); // "off", "cool", "fan_only"
  if (message == "off") { // TO REMOVE
    tadiran.setState(STATE_off);
    setLED(L_WHITE);
  } else if (message == "cool") {
    tadiran.setState(STATE_on);
    tadiran.setMode(MODE_cold);
    setLED(L_BLUE);
  } else if (message == "fan_only") {
    tadiran.setState(STATE_on);
    tadiran.setMode(MODE_fan);
    setLED(L_GREEN);
  } else if (message == "heat") {
    tadiran.setState(STATE_on);
    tadiran.setMode(MODE_heat);
    setLED(L_RED);
  } else if (message == "dry") {
    tadiran.setState(STATE_on);
    tadiran.setMode(MODE_dry);
    setLED(0xffff00); // Yellow
  }

  tadiran.print();
  Serial.println();
  IrSender.sendRaw(tadiran.codes, TADIRAN_BUFFER_SIZE, 38);
}

void toggleACLight(const String& topic, const String& message) {
  light = !light;
  
  Serial.println();
  if (light) { 
    Serial.println("AC light ON");
    tadiran.codes[45] = CODE_high;
    if (tadiran.getState() == STATE_off) { setLED(L_WHITE); }
    else if (tadiran.getMode() == MODE_cold) { setLED(L_BLUE); }
    else if (tadiran.getMode() == MODE_fan) { setLED(L_GREEN); }
    else if (tadiran.getMode() == MODE_heat) { setLED(L_RED); }
    else if (tadiran.getMode() == MODE_dry) { setLED(0xffff00); } // Yellow
  }
  else { 
    Serial.println("AC light OFF");
    tadiran.codes[45] = CODE_filler;
    setLED(0x000000);
  }

  Serial.println();
  IrSender.sendRaw(tadiran.codes, TADIRAN_BUFFER_SIZE, 38);
}

void setLED(crgb_t color){
  if (light) {
    myLED.setPixel( 0, color, 1 );
  } else {
    myLED.setPixel( 0, color, 1 );
    delay(100);
    myLED.setPixel( 0, 0x000000, 1 );
  }
}
