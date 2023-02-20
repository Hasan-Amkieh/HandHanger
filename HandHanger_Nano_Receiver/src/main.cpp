#include <Arduino.h>
#include <IRremote.h>

const uint16_t kIrReceiverPin = 2;  // The pin that the IR receiver is connected to
IRrecv irrecv(kIrReceiverPin);
decode_results results;

static const int moveCommand = 100;

void setup() {

  Serial.begin(9600);
  irrecv.enableIRIn();
  pinMode(3, OUTPUT);
  digitalWrite(3, LOW);

}

void loop() {
  if (irrecv.decode(&results)) {
    if (results.decode_type == NEC) {
      Serial.println(results.value, HEX);
      if (results.value == moveCommand) {
        digitalWrite(3, HIGH);
        delay(10);
        digitalWrite(3, LOW);
      }
    }
    irrecv.resume();
  }
}