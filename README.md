# Arduino Multithreading
## Basic Usage
1. include the scheduler.h file
2. call `setup_multithreading()` once at the start
3. create new tasks with `new_task()`
## Example
``` C++
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
```
## Functions
### setup_multithreading
 + __tick_length__: the time interval in which to check if the task should be switched in 1/100s
### new_task
 + __task_func_ptr__: a function pointer to the threads main method
 + __stack_size__: the size of the stack for this thread
 + __priority__: how much ticks a thread should be allowed to run for before switching to the next one 
## Global Variables
 + __task_locked__ is a global bool allowing you to lock the current thread
 + __current_task__ is a global struct of type thread_t containing the info of the current task and allows access to all existing threads by reading the other elements of the queue through the pointer `next`
