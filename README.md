# ESPL Exercise II and III

This repository contains my solution to the excercise 2 and 3 of the ESPL course.

## Functionality

There are three screens that can be toggled by pressing the **button E**.
Press Q to exit the program.

## Exercise II

* The number of times the buttons A, B, C and D are pressed is counted.  
* You can **press the mouse wheel** to reset the numbers.


## Exercise III

### Part 1
* The number of times the **buttons N and M** are pressed is counted.
* Press **button X** to start and stop the counter

### Part 2
There is no input. Also it's important to note, that the code is only called once (if you switch to the screen for the first time).

## Answers to the Questions
* How does the mouse guarantee thread-safe functionality?  
  -> Inside the struct holding the information for the mouse there is also a mutex to lock it. If e.g. tumEventGetMouseMiddle() is called, this function takes the mutex first, then it reads from the struct and then it gives the mutex back.
* What is the kernel tick?  
  -> The kernel tick is the frequency at which the scheduler is running. It is usually generated from a hardware timer.  
* What is a tickless kernel?  
  -> Usually the scheduler is periodically invoked as well as after a running task changes its state (to blocked or suspended). In a tickless kernel, this periodical invoking does not happen. The scheduler is only invoked if a running task changes its state.  
* What happens if the stack size is too low?  
  -> If the stack size is too low, stack overflows can happen which means that the program is writing in a memory region it is not supposed to write. If this region is not beeing used by anything else, you won't even notice it. However, if this region is used by some other program then very weird stuff can happen which may even lead to a complete failure of the OS.  
* Playing around with priorities - observations:
  * No matter what: Task 3 is always running after task 2 because it blocks on the  semaphore that's only given by task 2. Even if task 3 has the highest priority it's still running after task 2 and task 2 is running at its priority because semaphores don't feature priority inheritence.
  * The other tasks run in the order of their priority.
  * If the drawing task has a too low priority, the program does no longer work as intended because it's not able to fetch the notification after each task has been running. For sure, it would be possible to redesign the code in a way that it can read multiple notifications at a time. However, the information of the order would be lost. Therefore, I decided to leave it like this so it's obvious if the task is not executing at a priority that is high enough.