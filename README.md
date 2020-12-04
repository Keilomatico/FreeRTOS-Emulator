# ESPL Exercise II and III

This repositoy contains my solution to the excercise 2 and 3 of the ESPL course

## Functionality

There are three screens that can be toggled by pressing the **button E**.
Press Q to exit the program.

## Exercise II

* The number of times the buttons A, B, C and D are pressed is counted.  
* You can **press the mouse wheel** to reset the numbers.  

## Exercise III

### Answers to the Questions
* What is the kernel tick?  
  -> The kernel tick is the frequency at which the scheduler is running. It is usually generated from a hardware timer.  
* What is a tickless kernel?  
  -> Usually the scheduler is periodically invoked as well as after a running task changes its state (to blocked or suspended). In a tickless kernel, this periodical invoking does not happen. The scheduler is only invoked if a running task changes its state.  
* What happens if the stack size is too low?  
  -> If the stack size is too low stack overflows can happen which means that the program is writing in a memory region it is not supposed to write. If this region is not beeing used by anything else, you won't even notice it. However, if this region is used by some other program then very weird stuff can happen which may even lead to a complete failure of the OS.

### Part 1
* The number of times the **buttons N and M** are pressed is counted.
* Press **button X** to start and stop the counter

### Part 2
Not implmented yet.