#include "scheduler.h"
#include <string.h>
#include <stdlib.h>
#include <avr/interrupt.h>

void task_exit() {
  cli(); // disable interupts
  task_t* last = current_task;
  const task_t* next = current_task->next;
  while (last->next != current_task) { // find task in queue that is before the current one
    last = last->next;
  }
  last->next = current_task->next; // remove current task from queue
  free(current_task->stack_begin);
  free(current_task);


  current_task = next; // select next task
  SP = (uint16_t)current_task->stack_ptr; // restore stack pointer from new task

  asm volatile( // restore status register from stack but clear global interrupt flag to keep interrupts disabled
    "pop r16\n\t"
    "andi r16, 0x7f\n\t"
    "out __SREG__, r16\n\t"
  );
  asm volatile( // restore all general registers from stack
    "pop r31\n\tpop r30\n\tpop r29\n\tpop r28\n\tpop r27\n\tpop r26\n\tpop r25\n\tpop r24\n\t"
    "pop r23\n\tpop r22\n\tpop r21\n\tpop r20\n\tpop r19\n\tpop r18\n\tpop r17\n\tpop r16\n\t"
    "pop r15\n\tpop r14\n\tpop r13\n\tpop r12\n\tpop r11\n\tpop r10\n\tpop r09\n\tpop r08\n\t"
    "pop r07\n\tpop r06\n\tpop r05\n\tpop r04\n\tpop r03\n\tpop r02\n\tpop r01\n\tpop r00\n\t"
  );
  asm volatile( // return to new task. sets interrupt flag in status register enabling interrupts
    "reti\n\t"
  );
}

void new_task(const void (*task_func_ptr)(), uint8_t stack_size = 32, uint8_t priority = 1) { // TODO add posibility to set void* as task param
  cli(); // disable interupts
  task_t* new_next_task = malloc(sizeof(task_t)); // get space for data associated with task
  new_next_task->next = current_task->next; // add new task to queue
  current_task->next = new_next_task;

  stack_size += 37; // add space for fake stack frame
  void* stack_begin = malloc(stack_size); // get space for stack and init it
  memset(stack_begin, 0, stack_size);
  new_next_task->stack_begin = stack_begin;
  void* stack = stack_begin + stack_size;

  //setup fake stack frame
  uint16_t* func_addr_ptr = stack - 3;
  uint16_t* exit_addr_ptr = stack - 1;
  *func_addr_ptr = ((uint16_t)task_func_ptr << 8) | ((uint16_t)task_func_ptr >> 8);  // set stack entry for first return address to the function for the new task
  *exit_addr_ptr = ((uint16_t)&task_exit << 8) | ((uint16_t)&task_exit >> 8);  // set stack entry for for second return address to task_exit to exit the task gracefully

  new_next_task->stack_ptr = stack - 37; // set stack_ptr for the new task so it will take the fake stack frame

  new_next_task->ticks = 0;
  new_next_task->priority = priority;
  sei(); // enable interupts
}

// interrupt function without use of ISR macro to keep gcc from generating any additional code for it. todo: check if ISR_NAKED might also do the trick?
extern "C" void TIMER1_COMPA_vect (void) __attribute__ ((used, externally_visible));
void TIMER1_COMPA_vect (void) {
  cli(); // disable interupts
  asm volatile( // save all general registers to stack
    "push r00\n\tpush r01\n\tpush r02\n\tpush r03\n\tpush r04\n\tpush r05\n\tpush r06\n\tpush r07\n\t"
    "push r08\n\tpush r09\n\tpush r10\n\tpush r11\n\tpush r12\n\tpush r13\n\tpush r14\n\tpush r15\n\t"
    "push r16\n\tpush r17\n\tpush r18\n\tpush r19\n\tpush r20\n\tpush r21\n\tpush r22\n\tpush r23\n\t"
    "push r24\n\tpush r25\n\tpush r26\n\tpush r27\n\tpush r28\n\tpush r29\n\tpush r30\n\tpush r31\n\t"
  );
  asm volatile( // save status register to stack
    "in r16, __SREG__\n\t"
    "push r16\n\t"
  );

  if (++current_task->ticks >= current_task->priority && !task_locked) { // check if current task should switch
    current_task->stack_ptr = SP; // save current stack pointer

    current_task = current_task->next; // select next task

    SP = (uint16_t)current_task->stack_ptr; // restore stack pointer from new task
  }


  asm volatile( // restore status register from stack but clear global interrupt flag to keep interrupts disabled
    "pop r16\n\t"
    "andi r16, 0x7f\n\t"
    "out __SREG__, r16\n\t"
  );
  asm volatile( // restore all general registers from stack
    "pop r31\n\tpop r30\n\tpop r29\n\tpop r28\n\tpop r27\n\tpop r26\n\tpop r25\n\tpop r24\n\t"
    "pop r23\n\tpop r22\n\tpop r21\n\tpop r20\n\tpop r19\n\tpop r18\n\tpop r17\n\tpop r16\n\t"
    "pop r15\n\tpop r14\n\tpop r13\n\tpop r12\n\tpop r11\n\tpop r10\n\tpop r09\n\tpop r08\n\t"
    "pop r07\n\tpop r06\n\tpop r05\n\tpop r04\n\tpop r03\n\tpop r02\n\tpop r01\n\tpop r00\n\t"
  );
  asm volatile( // return to new task. sets interrupt flag in status register enabling interrupts
    "reti\n\t"
  );

}

void setup_multithreading(uint8_t tick_length = 25) {
  cli(); // disable interupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0; //initialize counter value to 0
  OCR1A = 0xA4 * tick_length; // set compare value for selected tick_length
  TCCR1B |= (1 << WGM12); // turn on CTC mode
  TCCR1B |= (1 << CS12) |  (1 << CS10); // set prescaler to 1024
  TIMSK1 |= (1 << OCIE1A); // enable timer interrupt
  sei(); // enable interupts
}


task_t main_task = {
  .next = &main_task,
  .stack_begin = NULL,
  .stack_ptr = NULL,
  .ticks = 0,
  .priority = 1
};

task_t* current_task = &main_task;
bool task_locked = true;
