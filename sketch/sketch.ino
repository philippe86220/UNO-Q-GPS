#include "trame.h"
#include <Arduino.h>
#include <Arduino_RouterBridge.h>

constexpr byte NMEA_MAX = 82;
char sentence[NMEA_MAX + 1];
Trame gps(nullptr);


void setup() {
  Bridge.begin();
  Serial.begin(9600);
  Monitor.begin();
  
}

bool getSentence() {
  static byte i = 0;
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r')              continue;
    if (c == '\n') {
      sentence[i] = '\0';
      i = 0;
      return true;
    }
    if (i < NMEA_MAX) sentence[i++] = c;
  }
  return false;
}

void loop() {

  
    
  if (getSentence()) gps.setSentence(sentence); 
    
  if (gps.extrait() && gps.estValide()) {
    Bridge.call("update_gps", gps.latitude, gps.longitude, gps.jour, gps.mois, gps.annee+2000,(gps.heure) % 24, gps.minute, gps.seconde);
   }
 
}
