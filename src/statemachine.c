#include "statemachine.h"

// ########### NOTES ############
// This statemachine follows the idea of a linked list

struct state_machine {
	state_parameters_t *head;	//Pointer to the first state

	unsigned int _state_count;	//Total number of states

	state_parameters_t *current;	//Pointer to the currently active state
} sm = { 0 };	//All values are initialized with 0

//Queue which contains the IDs of each state
QueueHandle_t state_queue = NULL;

unsigned int initState(state_parameters_t *state_params, void (*init)(void *), void (*enter)(void *),
		      void (*run)(void *), void (*exit)(void *), void *data)
{
    printf("Starting initState \n");
    state_parameters_t *iterator;

    state_params->init = init;
    state_params->enter = enter;
    state_params->run = run;
    state_params->exit = exit;
    state_params->data = data;

    if(!sm.head) {
        printf("sm.head points to Null; Inititalizing it with the current state. \n");
        sm.head = state_params;
        sm.current = sm.head;
    }
    else {
        //Go through all states until you reach the end of the list (iterator->next==0)
	    for (iterator = sm.head; iterator->next; iterator = iterator->next);
        iterator->next = state_params;
    }


    if (state_params->init)	//Check if the current state has an init function. If yes, call it
        //Remember: Init is a function pointer, so this is just calling the 
        //init function with the data as an argument
        (state_params->init)(state_params->data);

	//Calculate and return the state ID
    state_params->_ID = sm._state_count;
    sm._state_count++;
    printf("Finished initState \n");
    return (state_params->_ID);
}

state_parameters_t *findState(unsigned int ID)
{
	state_parameters_t *iterator;

	//Go through all states until you either reach the end (terator->next == NULL) 
    //or find the ID (iterator->next->_ID == ID)
	for (iterator = sm.head; iterator->next && (iterator->next->_ID != ID);
	     iterator = iterator->next)
		;

	return iterator->next;
}

void deleteState(unsigned int ID)
{
	state_parameters_t *iterator, *delete;

	//Same as in findState (I wonder why findState is not simply called here)
	for (iterator = sm.head; iterator->next && (iterator->next->_ID != ID);
	     iterator = iterator->next)
		;

    //Make sure that the task was actually found
    //(that iterator->next is not NULL)
	if (iterator->next)
		//Don't know what that next if should do.
		//iterator->next->_ID can never be 0 here 
        //(except if someone passed 0 to the function but who would do that?!)... 
		//Maybe just extra security
		if (iterator->next->_ID) {
			delete = iterator->next;

			//If the state which should be deleted is the last one in the list:
			//Set the next pointer of the previous element to 0
			//Otherwise: Reconnect the linked list
			if (!iterator->next->next)
				iterator->next = NULL;
			else
				iterator->next = delete->next;

			//free(delete);
		}
}

void vStatesHandler(void *pvParameters)
{
	unsigned char state_in = 0;

	TickType_t prev_wake_time;
	TickType_t last_change = 0;

    printf("StatesHandler started \n");

    //Create the queue which can hold all the state IDs
	state_queue = xQueueCreate(MAX_NUMBER_OF_STATES,
				   sizeof(unsigned int));

	if (!state_queue) {
		fprintf(stderr, "State queue creation failed\n");
		exit(EXIT_FAILURE);
	}

    (sm.current->enter)(sm.current->data);

	while (1) {
		xGetButtonInput(); // Update global input
		if(checkbutton(&last_change, KEYCODE(E))) {
			printf("Button E pressed \n");
			//Call the exit function of the current task if it exists
			if (sm.current->exit)
				(sm.current->exit)(sm.current->data);

			if(state_in >= sm._state_count - 1)
			{
				state_in = 0;
				sm.current = sm.head;
			}
			else
			{
				state_in++;
				sm.current = sm.current->next;
			}
			

			if (sm.current->enter)
				(sm.current->enter)(sm.current->data);
			
			//Call the run function of the (new) current task
			if (sm.current->run)
				(sm.current->run)(sm.current->data);
		}
		/*if (xQueueReceive(state_queue, &state_in, 0) == pdTRUE)
			sm.next = findState(state_in);
		*/

		vTaskDelayUntil(&prev_wake_time, STATE_MACHINE_INTERVAL);
	}
}