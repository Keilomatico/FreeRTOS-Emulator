#include <stdlib.h>
#include <stdio.h>

#define STATE_MACHINE_STATE_QUEUE_LENGTH 10
#define STATE_MACHINE_INTERVAL 10

// ########### NOTES ############
// This statemachine follows the idea of a linked list
//

//Stuct storing data for each individual system state
typedef struct system_state {
	unsigned int _ID;	//ID of the state

	void *data;		//Pointer to the data of the state

	void (*init)(void *);	//Pointer to the init function of the state
	void (*enter)(void *);	//Pointer to the enter function of the state
	void (*run)(void *);	//Pointer to the run function of the state
	void (*exit)(void *);	//Pointer to the exit function of the state

	//Pointer to the struct of the next state
	struct system_state *next;

	//Bitfield of length one (equivalent to a boolean in usage but more storage efficient)
	//Indicates if the state has been inititalized yet
	//See https://docs.microsoft.com/en-us/cpp/c-language/c-bit-fields
	unsigned char _initialized : 1;
} system_state_t;

//Struct for the whole statemachine
struct state_machine {
	system_state_t head;	//Struct for the first state

	unsigned int _state_count;	//Total number of states

	system_state_t *current;	//Pointer to the currently active state
	system_state_t *next;		//Pointer to the next state

	unsigned char _initialized : 1;
} sm = { 0 };	//All values are initialized with 0

QueueHandle_t state_queue = NULL;	//Queue from FreeRTOS

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
		      void (*run)(void *), void (*exit)(void *), void *data)
{
	system_state_t *iterator;

	//Go through all states until you reach the end of the list (iterator->next==0) (0 because we use calloc instead of malloc)
	for (iterator = &sm.head; iterator->next; iterator = iterator->next);

	iterator->next = (system_state_t *)calloc(1, sizeof(system_state_t));	//https://www.geeksforgeeks.org/difference-between-malloc-and-calloc-with-examples/ 
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

/**
 * @brief Finds the struct of a state by its ID
 *
 * @param ID ID of the state you are looking for
 * @return Struct of the state
 */
system_state_t *findState(unsigned int ID)
{
	system_state_t *iterator;

	//Go through all states until you either reach the end (terator->next == NULL) or find the ID (iterator->next->_ID == ID)
	for (iterator = &sm.head; iterator->next && (iterator->next->_ID != ID);
	     iterator = iterator->next)
		;

	return iterator->next;
}

/**
 * @brief Deletes a state
 *
 * @param ID ID of the state to be deleted
 */
void deleteState(unsigned int ID)
{
	system_state_t *iterator, *delete;

	//Same as in findState (I wonder why findState is not simply called here)
	for (iterator = &sm.head; iterator->next && (iterator->next->_ID != ID);
	     iterator = iterator->next)
		;

	if (iterator->next)	//Make sure that the task was actually found (that iterator->next is not NULL)
		//Don't know what that should do.
		//iterator->next->_ID can never be 0 here (except if someone passed 0 to the function but who would do that?!)... 
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
	system_state_t *iterator;

	//Create the queue which can hold all the state IDs
	state_queue = xQueueCreate(STATE_MACHINE_STATE_QUEUE_LENGTH,
				   sizeof(unsigned int));

	if (!state_queue) {
		fprintf(stderr, "State queue creation failed\n");
		exit(EXIT_FAILURE);
	}

	for (iterator = &sm.head; iterator->next; iterator = iterator->next)	//Go through the whole linked list
		if (iterator->init)	//Check if the current state has an init function
			(iterator->init)(iterator->data);	//Remember: Init is a function pointer, so this is just calling the init function with the data as an argument

	return 0;
}

void statesHandlerTask(void)
{
	unsigned char state_in;

	TickType_t prev_wake_time;

	//equiv. to
	//if(!sm.initialized) {
	//	sm.initialized++ (which just means sm.initialized=1;)
	if (!(sm._initialized++))
		if (!smInit && !(sm.current = sm.next = sm.head.next)) {	//Assigns the value of sm.head.next to sm.next and sm.current and checks if sm.head.next == 0
			fprintf(stderr, "No states\n");
			exit(EXIT_FAILURE);
		}

	while (1) {
		if (xQueueReceive(sm_queue, &state_in, 0) == pdTRUE)	//Should be state_queue
			sm.next = findState(state_in);

		//If the state was changed
		//Call the exit function of the current task and the enter function of the next task if they exist
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