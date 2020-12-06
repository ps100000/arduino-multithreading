#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include <stdint.h>

struct task_t {
  task_t* next;
  void* stack_begin;
  void* stack_ptr;
  uint8_t ticks;
  uint8_t priority;
};

// setup multithreading
// tick_length: the time interval in which to check if the task should be switched in 1/100s
void setup_multithreading(uint8_t tick_length = 25);
// create a new task
// task_func_ptr: a function pointer to the thread main method
// stack_size: the size of the stack for this thread
// priority: how much ticks a thread should be allowed to run for before switching to the next one 
void new_task(const void (*task_func_ptr)(), uint8_t stack_size = 32, uint8_t priority = 1);



extern task_t* current_task;
extern bool task_locked;

#endif
