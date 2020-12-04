#include "global.h"
#include "miscFunc.h"
#include "statemachine.h"
#include "exercise2.h"
#include "exercise3.h"
#include "exercise4.h"

#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)
#define mainGENERIC_STACK_SIZE ((unsigned short)2560)
#define TASK3B_STACK_SIZE 100

//Init with NULL, so you can check if it has been initialized
TaskHandle_t Exercise2 = NULL; 
TaskHandle_t Exercise3draw = NULL; 
TaskHandle_t Exercise3circle1 = NULL; 
TaskHandle_t Exercise3circle2 = NULL; 
TaskHandle_t Exercise3button1 = NULL; 
TaskHandle_t Exercise3button2 = NULL; 
TaskHandle_t Exercise3count = NULL; 
TaskHandle_t Exercise4 = NULL;
TaskHandle_t BufferSwap = NULL;
TaskHandle_t StatesHandler = NULL;

StaticTask_t Exercise3b_Buffer;
StackType_t task3bStack[ TASK3B_STACK_SIZE ];

/**
 * Draw Signal is used to synchronize vSwapBuffers and the task
 * currently drawing to the screen.
 * vSwapBuffers gives the semaphore once it's finished
 * and the drawing task is supposed to check take it, 
 * do the drawing and don't give it back.
 */ 
SemaphoreHandle_t DrawSignal  = NULL;
//Guards the screen buffer
SemaphoreHandle_t ScreenLock = NULL;
//Mutexes to make sure the tasks finish before they are suspended
SemaphoreHandle_t exercise2Mutex = NULL;
SemaphoreHandle_t exercise3Mutex = NULL;
SemaphoreHandle_t exercise4Mutex = NULL;
//Semaphore to notify Exercise3button1 that the button has been pressed 
SemaphoreHandle_t button1Notify = NULL;

//Queue Handles for the number of times the buttons have been presses (in ex 3)
QueueHandle_t button1Num = NULL;
QueueHandle_t button2Num = NULL;
//Queue Handle for the number value of the counter (in ex 3)
QueueHandle_t counterVal = NULL;

TimerHandle_t Exercise3timer = NULL;

buttons_buffer_t buttons = { 0 };

/**
 * These are the structs holding the parameters for the individual states.
 * They are initialized before the scheduler starts by calling initState 
 * and then only read by the statemachine.
 * Do I need to lock these?
*/
state_parameters_t state_param_ex2 = { 0 };
state_parameters_t state_param_ex3 = { 0 };
state_parameters_t state_param_ex4 = { 0 };

//See https://www.freertos.org/a00110.html
//Necessary, because configSUPPORT_STATIC_ALLOCATION is set to 1
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
//Necessary, because static allocation and timers are enabled
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize )
{
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

/**
 * @brief Task to swap the buffer of the screen to have a smooth user experience
*/
void vSwapBuffers(void *pvParameters)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    const TickType_t frameratePeriod = 1000 / FRAMERATE;

    tumDrawBindThread(); // Setup Rendering handle with correct GL context

    while (1) {
        if (xSemaphoreTake(ScreenLock, portMAX_DELAY) == pdTRUE) {
            tumDrawUpdateScreen();
            tumEventFetchEvents(FETCH_EVENT_BLOCK);
            xSemaphoreGive(ScreenLock);
            xSemaphoreGive(DrawSignal);
            vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(frameratePeriod));
        }
    }
}

int main(int argc, char *argv[])
{
    //Required for images
    char *bin_folder_path = tumUtilGetBinFolderPath(argv[0]);
    
    printf("Initializing: ");

    // Initialization
    if (tumDrawInit(bin_folder_path)) {
        PRINT_ERROR("Failed to initialize drawing");
        goto err_init_drawing;
    }
    if (tumEventInit()) {
        PRINT_ERROR("Failed to initialize events");
        goto err_init_events;
    }
    if (tumSoundInit(bin_folder_path)) {
        PRINT_ERROR("Failed to initialize audio");
        goto err_init_audio;
    }

    //Generate all the Semaphores and Mutexes
    //If something fails, go to the appropriate error to delete them again
    buttons.lock = xSemaphoreCreateMutex();
    if (!buttons.lock) {
        PRINT_ERROR("Failed to create buttons lock");
        goto err_buttons_lock;
    }
    DrawSignal = xSemaphoreCreateBinary();
    if (!DrawSignal) {
        PRINT_ERROR("Failed to create draw signal");
        goto err_draw_signal;
    }
    ScreenLock = xSemaphoreCreateMutex();
    if (!ScreenLock) {
        PRINT_ERROR("Failed to create screen lock");
        goto err_screen_lock;
    }
    //Use mutexes for the exercises to allow for priority inheritance
    exercise2Mutex = xSemaphoreCreateMutex();
    if (!exercise2Mutex) {
        PRINT_ERROR("Failed to create exercise2 semaphore");
        goto err_ex2_sem;
    }
    exercise3Mutex = xSemaphoreCreateMutex();
    if (!exercise3Mutex) {
        PRINT_ERROR("Failed to create exercise3 semaphore");
        goto err_ex3_sem;
    }
    button1Notify = xSemaphoreCreateBinary();
    if (!button1Notify) {
        PRINT_ERROR("Failed to create button1Notify semaphore");
        goto err_button1Notify;
    }
    button1Num = xQueueCreate(1, sizeof(unsigned int));
    if (!button1Num) {
        PRINT_ERROR("Failed to create button1Num queue");
        goto err_button1Num;
    }
    button2Num = xQueueCreate(1, sizeof(unsigned int));
    if (!button2Num) {
        PRINT_ERROR("Failed to create button2Num semaphore");
        goto err_button2Num;
    }
    counterVal = xQueueCreate(1, sizeof(unsigned int));
    if (!counterVal) {
        PRINT_ERROR("Failed to create counter semaphore");
        goto err_counterVal;
    }
    exercise4Mutex = xSemaphoreCreateMutex();
    if (!exercise4Mutex) {
        PRINT_ERROR("Failed to create exercise4 semaphore");
        goto err_ex4_sem;
    }
    
    //Create all the tasks and the timer
    //If something fails, go to the appropriate error to delete them again
    if (xTaskCreate(vSwapBuffers, "BufferSwapTask",
                    mainGENERIC_STACK_SIZE * 2, NULL, configMAX_PRIORITIES,
                    &BufferSwap) != pdPASS) {
        goto err_bufferswap;
    }
    if (xTaskCreate(vExercise2, "Exercise2", mainGENERIC_STACK_SIZE * 2, NULL,
                    mainGENERIC_PRIORITY, &Exercise2) != pdPASS) {
        goto err_exercise2;
    }
    if (xTaskCreate(vExercise3draw, "Exercise3draw", mainGENERIC_STACK_SIZE * 2, NULL,
                    mainGENERIC_PRIORITY, &Exercise3draw) != pdPASS) {
        goto err_exercise3_draw;
    }
    if (xTaskCreate(vExercise3circle1, "Exercise3circle1", mainGENERIC_STACK_SIZE * 2, NULL,
                    mainGENERIC_PRIORITY, &Exercise3circle1) != pdPASS) {
        goto err_exercise3circle1;
    }
    Exercise3circle2 = xTaskCreateStatic(vExercise3circle2, "Exercise3circle2", TASK3B_STACK_SIZE,
                      NULL, mainGENERIC_PRIORITY+1, task3bStack, &Exercise3b_Buffer);
    if (!Exercise3circle2) {
        goto err_exercise3circle2;
    }
    if (xTaskCreate(vExercise3button1, "Exercise3button1", mainGENERIC_STACK_SIZE * 2, NULL,
                    mainGENERIC_PRIORITY, &Exercise3button1) != pdPASS) {
        goto err_exercise3button1;
    }
    if (xTaskCreate(vExercise3button2, "Exercise3button2", mainGENERIC_STACK_SIZE * 2, NULL,
                    mainGENERIC_PRIORITY+1, &Exercise3button2) != pdPASS) {
        goto err_exercise3button2;
    }
    if (xTaskCreate(vExercise3count, "Exercise3count", mainGENERIC_STACK_SIZE * 2, NULL,
                    mainGENERIC_PRIORITY, &Exercise3count) != pdPASS) {
        goto err_exercise3count;
    }
    Exercise3timer = xTimerCreate("Exercise3count", COUNTER_RESET, pdTRUE,
                    ( void * ) 0, vExercise3timerCallback);
    if(!Exercise3timer)
        goto err_exercise3timer;
    if (xTaskCreate(vExercise4, "Exercise4", mainGENERIC_STACK_SIZE * 2, NULL,
                    mainGENERIC_PRIORITY, &Exercise4) != pdPASS) {
        goto err_exercise4;
    }
    if (xTaskCreate(vStatesHandler, "StatesHandler", 
                    mainGENERIC_STACK_SIZE * 2, NULL, configMAX_PRIORITIES-2,
                    &StatesHandler) != pdPASS) {
        goto err_statesHandler;
    }

    //Suspend all tasks for the exercises
    vTaskSuspend(Exercise2);
    vTaskSuspend(Exercise3draw);
    vTaskSuspend(Exercise3circle1);
    vTaskSuspend(Exercise3circle2);
    vTaskSuspend(Exercise3button1);
    vTaskSuspend(Exercise3button2);
    vTaskSuspend(Exercise3count);
    vTaskSuspend(Exercise4);
    //Note: The timer is created in a dormant state and thus doesn't need to be stopped here

    tumFUtilPrintTaskStateList();

    //Initialize states by the statemachine
    printf("Created state for Exercise 2 with ID %d. \n",
        initState(&state_param_ex2, NULL, exercise2enter, NULL, exercise2exit, NULL));
    printf("Created state for Exercise 3 with ID %d. \n",
        initState(&state_param_ex3, NULL, exercise3enter, NULL, exercise3exit, NULL));
    printf("Created state for Exercise 4 with ID %d. \n",
        initState(&state_param_ex4, NULL, exercise4enter, NULL, exercise4exit, NULL));

    //Example on how to delete states and what's happening with them
    /*state_parameters_t *test;
    test = findState(1);
    printf("test has ID %d (should be 1)\n", test->_ID);

    printf("Deleting state 1 \n");
    deleteState(1);

    test = findState(1);
    if(!test)
        printf("State 1 successfully deleted \n");
    
    printf("Created state for Exercise 3 with ID %d. \n",
        initState(&state_param_ex3, NULL, exercise3run, NULL, exercise3exit, NULL));
    */

    vTaskStartScheduler();

    return EXIT_SUCCESS;

//Error handling -> Deinitialisation of all the things that were created
err_statesHandler:
    vTaskDelete(Exercise4);
err_exercise4:
    xTimerDelete(Exercise3timer, portMAX_DELAY);
err_exercise3timer:
    vTaskDelete(Exercise3count);
err_exercise3count:
    vTaskDelete(Exercise3button2);
err_exercise3button2:
    vTaskDelete(Exercise3button1);
err_exercise3button1:
    vTaskDelete(Exercise3circle2);
err_exercise3circle2:
    vTaskDelete(Exercise3circle1);
err_exercise3circle1:
    vTaskDelete(Exercise3draw);
err_exercise3_draw:
    vTaskDelete(Exercise2);
err_exercise2:
    vTaskDelete(BufferSwap);
err_bufferswap:
    vSemaphoreDelete(exercise4Mutex);
err_ex4_sem:
    vSemaphoreDelete(exercise3Mutex);
err_ex3_sem:
    vSemaphoreDelete(button1Notify);
err_button1Notify:
    vQueueDelete(button1Num);
err_button1Num:
    vQueueDelete(button2Num);
err_button2Num:
    vQueueDelete(counterVal);
err_counterVal:
    vSemaphoreDelete(exercise2Mutex);
err_ex2_sem:
    vSemaphoreDelete(ScreenLock);
err_screen_lock:
    vSemaphoreDelete(DrawSignal);
err_draw_signal:
    vSemaphoreDelete(buttons.lock);
err_buttons_lock:
    tumSoundExit();
err_init_audio:
    tumEventExit();
err_init_events:
    tumDrawExit();
err_init_drawing:
    return EXIT_FAILURE;

    vSemaphoreDelete(state_param_ex2.lock);
}

//Just leave these things in here
// cppcheck-suppress unusedFunction
__attribute__((unused)) void vMainQueueSendPassed(void)
{
    /* This is just an example implementation of the "queue send" trace hook. */
}

// cppcheck-suppress unusedFunction
__attribute__((unused)) void vApplicationIdleHook(void)
{
#ifdef __GCC_POSIX__
    struct timespec xTimeToSleep, xTimeSlept;
    /* Makes the process more agreeable when using the Posix simulator. */
    xTimeToSleep.tv_sec = 1;
    xTimeToSleep.tv_nsec = 0;
    nanosleep(&xTimeToSleep, &xTimeSlept);
#endif
}
