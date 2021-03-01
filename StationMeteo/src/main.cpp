#include <Arduino.h>
#include "StationMeteo.h"

StationMeteo stationMeteo;

void setup() {
  // put your setup code here, to run once:
  stationMeteo.reset();
}

void loop() {
  stationMeteo.reconnect();
  stationMeteo.executerMqtt();
  stationMeteo.afficherDels();
  stationMeteo.afficherDelHumidite();
  stationMeteo.afficherLCD();
  delay(2000);
}