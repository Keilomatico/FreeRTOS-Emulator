/**
 * @file global.h
 * @author Adrian Keil
 * @date 02 December 2020
 * @brief This is the global header, which contains 
 * all globally shared variables and structures
 */

#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>

#include <SDL2/SDL_scancode.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "TUM_Ball.h"
#include "TUM_Draw.h"
#include "TUM_Event.h"
#include "TUM_Sound.h"
#include "TUM_Utils.h"
#include "TUM_Font.h"

#include "AsyncIO.h"

#define KEYCODE(CHAR) SDL_SCANCODE_##CHAR

#define DEBOUNCE_DELAY 300

/**
 * @brief Stores the state of each button on the keyboard
 * and has a Semaphore to be locked
 */
typedef struct buttons_buffer {
    unsigned char buttons[SDL_NUM_SCANCODES];
    SemaphoreHandle_t lock;
}buttons_buffer_t;

/**
 * @brief Stores data for each individual system state
 */
typedef struct state_parameters {
	unsigned int _ID;	//ID of the state

	void *data;		//Pointer to the data of the state

	void (*init)(void *);	//Pointer to the init function of the state
	void (*enter)(void *);	//Pointer to the enter function of the state
	void (*run)(void *);	//Pointer to the run function of the state
	void (*exit)(void *);	//Pointer to the exit function of the state

	//Pointer to the struct of the next state
	struct state_parameters *next;

	//Bitfield of length one (equivalent to a boolean in usage but more storage efficient)
	//Indicates if the state has been inititalized yet
	//See https://docs.microsoft.com/en-us/cpp/c-language/c-bit-fields
	unsigned char _initialized : 1;

    SemaphoreHandle_t lock;
} state_parameters_t;

extern TaskHandle_t Exercise2;
extern TaskHandle_t Exercise3;
extern TaskHandle_t Exercise4;
extern TaskHandle_t BufferSwap;
extern TaskHandle_t StatesHandler;

extern SemaphoreHandle_t DrawSignal;
extern SemaphoreHandle_t ScreenLock;
extern buttons_buffer_t buttons;

extern state_parameters_t state_param_ex2;
extern state_parameters_t state_param_ex3;
extern state_parameters_t state_param_ex4;


#endif