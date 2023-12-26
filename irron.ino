#include "Tadiran.h"
#include <IRremote.hpp>

const uint16_t kIrLed = 12;

// the IR emitter object
// IRsend irsend(kIrLed);
int incomingByte = 0;
// A/C Settings
int temperature = 26;
int mode = MODE_auto; // 0-Auto | 1-Cold | 2-Dry | 3-Fan | 4-Heat
int fanspeed = FAN_auto; //0-Auto | 1-Low | 2-Medium | 3-High
//A/C Toggles
boolean power = true;

// the Tairan code generator object
Tadiran tadiran(mode, fanspeed, temperature, power == 1 ? 1 : 0);

void setup()
{
  Serial.begin(115200);
  // irsend.begin(kIrLed);
  IrSender.begin(kIrLed);
  Serial.println("Commands: +/- Temperature | m - Mode | f - fanspeed | p - Power");

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
 if (Serial.available() > 0) {
        //Serial.write(27);  // ESC command
        //Serial.print("[2J");
        // read the incoming byte:
        incomingByte = Serial.read();
        Serial.println("Incoming Byte: " + incomingByte);
       
        // say what you got:
        Serial.print("Command: ");
        if(incomingByte == 43){
            Serial.print("Temperature +");
            if(temperature +1 <=30){
                temperature++;
            }
            tadiran.setTemeprature(temperature);
        }
        if(incomingByte == 45){
            Serial.print("Temperature -");
            if(temperature -1 >=16){
                temperature--;
            }
            tadiran.setTemeprature(temperature);
        }
        if(incomingByte == 109){ //mode
            Serial.print("Mode");
            if(mode+1 <=4) {
                mode++;
            } else {
                mode=0;
            }
            tadiran.setMode(mode);
        }
        if(incomingByte == 102) { //fan
            Serial.print("Fan");
            if(fanspeed+1 <=3) {
                fanspeed++;
            } else {
                fanspeed=0;
            }
            tadiran.setFan(fanspeed);
        }
        if(incomingByte == 112){ //power
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

