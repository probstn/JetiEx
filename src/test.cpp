#include <Arduino.h>
#include "JetiEx.h"

void setup() {
    Serial.begin(115200);
    ex_setup();
}

void loop() {
    ex_process();
}