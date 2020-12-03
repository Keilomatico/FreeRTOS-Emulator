#include "statemachine.h"

// ########### NOTES ############
// This statemachine follows the idea of a linked list

struct state_machine {
	state_parameters_t *head;	//Pointer to the first state

	unsigned int _state_count;	//Total number of states

	state_parameters_t *current;	//Pointer to the currently active state
} sm = { 0 };	//All values are initialized with 0

unsigned int initState(state_parameters_t *state_params, void (*init)(void *), void (*enter)(void *),
		      void (*run)(void *), void (*exit)(void *), void *data)
{
    state_parameters_t *iterator;
	int max_ID = 0;

	state_params->next = NULL;

    state_params->init = init;
    state_params->enter = enter;
    state_params->run = run;
    state_params->exit = exit;
    state_params->data = data;

    if(!sm.head) {
        printf("sm.head points to Null; Inititalizing it with the current state. \n");
        sm.head = state_params;
        sm.current = sm.head;
		state_params->_ID = 0;
    }
    else {
        //Go through all states until you reach the end of the list (iterator->next==0)
		//While iterating through this list also find the state with the largest ID
	    for (iterator = sm.head; iterator->next; iterator = iterator->next) {
			if(iterator->_ID > max_ID) {
				max_ID = iterator->_ID;
			}
		}
		//Don't forget to check the last item
		if(iterator->_ID > max_ID) {
			max_ID = iterator->_ID;
		}
		//Append the new state at the end of the linked list
        iterator->next = state_params;
		//Calculate ID of the new state
		state_params->_ID = max_ID + 1;
    }

    if (state_params->init)	//Check if the current state has an init function. If yes, call it
        //Remember: Init is a function pointer, so this is just calling the 
        //init function with the data as an argument
        (state_params->init)(state_params->data);
    
    sm._state_count++;
    return (state_params->_ID);
}

state_parameters_t *findState(unsigned int ID)
{
	state_parameters_t *iterator;

	//Go through all states until you either reach the end (iterator->next == NULL) 
    //or find the ID (iterator->next->_ID == ID)
	for (iterator = sm.head; iterator->next && (iterator->next->_ID != ID);
	     iterator = iterator->next)
		;

	if(iterator->next)
		return iterator->next;
	else
		return NULL;
}

void deleteState(unsigned int ID)
{
	state_parameters_t *iterator, *delete;

	//Go through all states until you either reach the end (iterator->next == NULL) 
    //or find the ID (iterator->next->_ID == ID)
	//So iterator->next is the task we are looking for
	for (iterator = sm.head; iterator->next && (iterator->next->_ID != ID);
	     iterator = iterator->next);

    //Make sure that the task was actually found
    //(that iterator->next is not NULL)
	if (iterator->next) {
		delete = iterator->next;

		//If the state which should be deleted is the last one in the list:
		//Set the next pointer of the previous element to 0
		//Otherwise: Reconnect the linked list
		if (!iterator->next->next)
			iterator->next = NULL;
		else
			iterator->next = delete->next;

		sm._state_count--;
	}
}

void vStatesHandler(void *pvParameters)
{
	//unsigned char state_in = 0;

	TickType_t prev_wake_time = xTaskGetTickCount();
	TickType_t last_change = 0;

    printf("StatesHandler started \n");

	//Enter the first state
    (sm.current->enter)(sm.current->data);

	while (1) {
		xGetButtonInput(); // Update global input
		if(checkbutton(&last_change, KEYCODE(E))) {
			printf("Button E pressed \n");
			//Call the exit function of the current task if it exists
			if (sm.current->exit)
				(sm.current->exit)(sm.current->data);

			if(!sm.current->next)
			{
				sm.current = sm.head;
			}
			else
			{
				sm.current = sm.current->next;
			}
			
			if (sm.current->enter)
				(sm.current->enter)(sm.current->data);
		}
		
		//Call the run function of the current task if it has one
		if (sm.current->run)
			(sm.current->run)(sm.current->data);

		vTaskDelayUntil(&prev_wake_time, STATE_MACHINE_INTERVAL);
	}
}