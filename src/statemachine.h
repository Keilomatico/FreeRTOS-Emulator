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
#include "miscFunc.h"

//After this number of ticks the state machine is wakened again
#define STATE_MACHINE_INTERVAL 10

/**
 * @brief Contains the data for the whole statemachine 
 * (including an instance of state_parameters for each state)
 */
struct state_machine;

/**
 * @brief Initializes a state with all its parameters
 *
 * @param init Function pointer to the init function
 * @param enter Function pointer to the enter function
 * @param run Function pointer to the run function
 * @param exit Function pointer to the exit function
 * @param data Data for the state
 * @return New state ID
 */
unsigned int initState(state_parameters_t *state_params, void (*init)(void *), void (*enter)(void *),
		      void (*run)(void *), void (*exit)(void *), void *data);

/**
 * @brief Finds the struct of a state by its ID
 *
 * @param ID ID of the state you are looking for
 * @return Struct of the state if it's found, NULL if not
 */
state_parameters_t *findState(unsigned int ID);

/**
 * @brief Deletes a state
 *
 * @param ID ID of the state to be deleted
 */
void deleteState(unsigned int ID);

/**
 * @brief Task which handles switching between states
 */
void vStatesHandler(void *pvParameter);

#endif