/**
 * @file statemachine.h
 * @author Adrian Keil
 * @brief State machine to handle advanced state switching
 */

#ifndef __STATEMACHINE_H__
#define __STATEMACHINE_H__

#include <stdlib.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "global.h"

#define STATE_MACHINE_STATE_QUEUE_LENGTH 10
#define STATE_MACHINE_INTERVAL 10

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
} state_parameters_t;

/**
 * @brief Contains the data for the whole statemachine 
 * (including an instance of state_parameters for each state)
 */
struct state_machine;

/**
 * @brief Adds a state to the statemachine
 *
 * @param init Function pointer to the init function
 * @param enter Function pointer to the enter function
 * @param run Function pointer to the run function
 * @param exit Function pointer to the exit function
 * @param data Data for the state
 * @return New state ID
 */
unsigned int addState(void (*init)(void *), void (*enter)(void *),
		      void (*run)(void *), void (*exit)(void *), void *data);

/**
 * @brief Finds the struct of a state by its ID
 *
 * @param ID ID of the state you are looking for
 * @return Struct of the state
 */
state_parameters_t *findState(unsigned int ID);

/**
 * @brief Deletes a state
 *
 * @param ID ID of the state to be deleted
 */
void deleteState(unsigned int ID);

/**
 * @brief Initializes the statemachine 
 * @return 0 on succes
 */
unsigned char smInit(void);

/**
 * @brief Task which handles switching between states
 */
void statesHandlerTask(void);

#endif