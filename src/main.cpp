#include <Arduino.h>
#include "sets.h"
#include <uButton.h>

#define PIN_FAN 4
#define PIN_BOOST_BTN 5
#define PIN_BOOST_LED 16
int curNormalSpeed;
bool isInBoost;
uint32_t boostTmr;

uButton btn(PIN_BOOST_BTN, INPUT_PULLUP);

void setSpeed(int val){
    int valPwm = map(val, 0, 100, 0, 255);
    analogWrite(PIN_FAN, valPwm);

    logger.printf("fan: %d\n", valPwm);
    Serial.printf("fan: %d\n", valPwm);
}

void normalSpeed_onChange(){
    curNormalSpeed = db[kk::normal_speed];
    setSpeed(curNormalSpeed);
}

void boost(){
    int val;
    if (isInBoost){
        isInBoost = false;
        val = db[kk::normal_speed];
        digitalWrite(PIN_BOOST_LED, 0);
    }
    else {
        isInBoost = true;
        val = db[kk::boost_speed];
        digitalWrite(PIN_BOOST_LED, 1);
        boostTmr = millis();
    }

    setSpeed(val);
}

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    pinMode(PIN_BOOST_LED, OUTPUT);
    digitalWrite(PIN_BOOST_LED, 0);

    sett_begin();

    delay(1000); //иначе не стартует wi-fi esp32-c3
}

void loop() {
    sett_loop();

    if (curNormalSpeed != (int)db[kk::normal_speed]){
        normalSpeed_onChange();
    }

    btn.tick();
    if (btn.release()){
        boost();
    }

    // finish boost
    if (isInBoost && (millis() - boostTmr) >= (uint32_t)db[kk::boost_duration] * 60000) {
        boost();
    }
}
