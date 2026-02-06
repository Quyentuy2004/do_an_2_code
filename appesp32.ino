#include "wifiConfig.h"

void setup() {
  Serial.begin(115200);
  wifiConfig.begin();
}
void loop() {
  wifiConfig.run();
}