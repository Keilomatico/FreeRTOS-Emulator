#include "statemachine.h"

// ########### NOTES ############
// This statemachine follows the idea of a linked list
// Difference between malloc and calloc:
// https://www.geeksforgeeks.org/difference-between-malloc-and-calloc-with-examples/ 

struct state_machine {
	state_parameters_t head;	//Struct for the first state

	unsigned int _state_count;	//Total number of states

	state_parameters_t *current;	//Pointer to the currently active state
	state_parameters_t *next;		//Pointer to the next state

	unsigned char _initialized : 1;
} sm = { 0 };	//All values are initialized with 0

//Create the handle for the statequeue
//This queue will later contain the IDs of each state
QueueHandle_t state_queue = NULL;

unsigned int addState(void (*init)(void *), void (*enter)(void *),
		      void (*run)(void *), void (*exit)(void *), void *data)
{
	state_parameters_t *iterator;

	//Go through all states until you reach the end of the list (iterator->next==0)
    // (0 because we use calloc instead of malloc)
	for (iterator = &sm.head; iterator->next; iterator = iterator->next);

	iterator->next = (state_parameters_t *)calloc(1, sizeof(state_parameters_t));
	iterator->next->init = init;
	iterator->next->enter = enter;
	iterator->next->run = run;
	iterator->next->exit = exit;
	iterator->next->data = data;

	//Return the new state ID
	//Could be also written like so:
	//sm._state_count += 1;
	//return (iterator->_ID = sm._state_count);
	return (iterator->_ID = ++sm._state_count);
}

state_parameters_t *findState(unsigned int ID)
{
	state_parameters_t *iterator;

	//Go through all states until you either reach the end (terator->next == NULL) 
    //or find the ID (iterator->next->_ID == ID)
	for (iterator = &sm.head; iterator->next && (iterator->next->_ID != ID);
	     iterator = iterator->next)
		;

	return iterator->next;
}

void deleteState(unsigned int ID)
{
	state_parameters_t *iterator, *delete;

	//Same as in findState (I wonder why findState is not simply called here)
	for (iterator = &sm.head; iterator->next && (iterator->next->_ID != ID);
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

			free(delete);
		}
}

unsigned char smInit(void)
{
	state_parameters_t *iterator;

	//Create the queue which can hold all the state IDs
	state_queue = xQueueCreate(STATE_MACHINE_STATE_QUEUE_LENGTH,
				   sizeof(unsigned int));

	if (!state_queue) {
		fprintf(stderr, "State queue creation failed\n");
		exit(EXIT_FAILURE);
	}

    //Go through the whole linked list
	for (iterator = &sm.head; iterator->next; iterator = iterator->next)	
		if (iterator->init)	//Check if the current state has an init function
			//Remember: Init is a function pointer, so this is just calling the 
            //init function with the data as an argument
            (iterator->init)(iterator->data);

	return 0;
}

void statesHandlerTask(void)
{
	unsigned char state_in;

	TickType_t prev_wake_time;

	//equiv. to
	//if(!sm.initialized) {
	//	sm.initialized++ (which just means sm.initialized=1;)
	if (!(sm._initialized++)) {
        smInit();
        //Assign the value of sm.head.next to sm.next and sm.current and checks if sm.head.next == 0
		if (!(sm.current = sm.next = sm.head.next)) {
			fprintf(stderr, "No states\n");
			exit(EXIT_FAILURE);
		}
    }

	while (1) {
		if (xQueueReceive(state_queue, &state_in, 0) == pdTRUE)
			sm.next = findState(state_in);

		//If the state was changed
		//Call the exit function of the current task and the enter function
        // of the next task if they exist
		if (sm.current != sm.next) {
			if (sm.current->exit)
				(sm.current->exit)(sm.current->data);

			if (sm.next->enter)
				(sm.next->enter)(sm.next->data);

			sm.current = sm.next;	//Make the next task the current task
		}
		//Call the run function of the (new) current task
		if (sm.current->run)
			(sm.current->run)(sm.current->data);

		vTaskDelayUntil(&prev_wake_time, STATE_MACHINE_INTERVAL);
	}
}