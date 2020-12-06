#include "scheduler.h"
// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int led = 13;

void test() {
  for(int i = 0; i < 5; i++) {
    delay(250);
    digitalWrite(led, HIGH);
    delay(500);
    digitalWrite(led, LOW);
    Serial.println("test");
  }
}

// the setup routine runs once when you press reset:
void setup() {
  pinMode(led, OUTPUT);
  Serial.begin(38400);
  delay(1000);
  Serial.println("");
  Serial.println("---- START ----");
  Serial.println("");

  setup_multithreading(1);
  Serial.println("task interrupt set");
  new_task(test, 128);
  Serial.println("new task created");
  task_locked = false;

}

// the loop routine runs over and over again forever:
void loop() {
  Serial.println("2");
  delay(1000);
}
