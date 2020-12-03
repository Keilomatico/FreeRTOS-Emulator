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
TaskHandle_t Exercise3a = NULL;
TaskHandle_t Exercise3b = NULL;
TaskHandle_t Exercise4 = NULL;
TaskHandle_t BufferSwap = NULL;
TaskHandle_t StatesHandler = NULL;

StaticTask_t Exercise3b_Buffer;
StackType_t task3bStack[ TASK3B_STACK_SIZE ];

//Draw Signal is used to synchronize vSwapBuffers and the task
//currently drawing to the screen
//vSwapBuffers gives the semaphore once it's finished
//and the drawing task is supposed to check if it's one
//if yes take it, do the drawing and don't give it back
SemaphoreHandle_t DrawSignal  = NULL;
SemaphoreHandle_t ScreenLock = NULL;
SemaphoreHandle_t exercise2Mutex = NULL;
SemaphoreHandle_t exercise3aMutex = NULL;
SemaphoreHandle_t exercise3bMutex = NULL;
SemaphoreHandle_t exercise4Mutex = NULL;
buttons_buffer_t buttons = { 0 };

//Do I need to lock these?
static state_parameters_t state_param_ex2 = { 0 };
static state_parameters_t state_param_ex3 = { 0 };
static state_parameters_t state_param_ex4 = { 0 };

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

void vSwapBuffers(void *pvParameters)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    const TickType_t frameratePeriod = 20;

    tumDrawBindThread(); // Setup Rendering handle with correct GL context

    while (1) {
        if (xSemaphoreTake(ScreenLock, portMAX_DELAY) == pdTRUE) {
            vDrawFPS();
            tumDrawUpdateScreen();
            tumEventFetchEvents(FETCH_EVENT_BLOCK);
            xSemaphoreGive(ScreenLock);
            xSemaphoreGive(DrawSignal);
            vTaskDelayUntil(&xLastWakeTime,
                            pdMS_TO_TICKS(frameratePeriod));
        }
    }
}

int main(int argc, char *argv[])
{
    /* Just leave all this stuff in here and don't think about it */
    char *bin_folder_path = tumUtilGetBinFolderPath(argv[0]); //Required for images

    printf("Initializing: ");

    /* Initialization (mostly of the semaphores/mutexes) */
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

    buttons.lock = xSemaphoreCreateMutex(); // Locking mechanism
    if (!buttons.lock) {
        PRINT_ERROR("Failed to create buttons lock");
        goto err_buttons_lock;
    }
    DrawSignal = xSemaphoreCreateBinary(); // Screen buffer locking
    if (!DrawSignal) {
        PRINT_ERROR("Failed to create draw signal");
        goto err_draw_signal;
    }
    ScreenLock = xSemaphoreCreateMutex();
    if (!ScreenLock) {
        PRINT_ERROR("Failed to create screen lock");
        goto err_screen_lock;
    }
    //Use mutexes for the exercise Semaphores to allow for priority inheritance
    exercise2Mutex = xSemaphoreCreateMutex();
    if (!exercise2Mutex) {
        PRINT_ERROR("Failed to create exercise2 semaphore");
        goto err_ex2_sem;
    }
    exercise3aMutex = xSemaphoreCreateMutex();
    if (!exercise3aMutex) {
        PRINT_ERROR("Failed to create exercise3 semaphore");
        goto err_ex3a_sem;
    }
    exercise3bMutex = xSemaphoreCreateMutex();
    if (!exercise3bMutex) {
        PRINT_ERROR("Failed to create exercise3 semaphore");
        goto err_ex3b_sem;
    }
    exercise4Mutex = xSemaphoreCreateMutex();
    if (!exercise4Mutex) {
        PRINT_ERROR("Failed to create exercise4 semaphore");
        goto err_ex4_sem;
    }
    
    if (xTaskCreate(vSwapBuffers, "BufferSwapTask",
                    mainGENERIC_STACK_SIZE * 2, NULL, configMAX_PRIORITIES,
                    &BufferSwap) != pdPASS) {
        goto err_bufferswap;
    }
    if (xTaskCreate(vExercise2, "Exercise2", mainGENERIC_STACK_SIZE * 2, NULL,
                    mainGENERIC_PRIORITY, &Exercise2) != pdPASS) {
        goto err_exercise2;
    }
    if (xTaskCreate(vExercise3a, "Exercise3", mainGENERIC_STACK_SIZE * 2, NULL,
                    mainGENERIC_PRIORITY, &Exercise3a) != pdPASS) {
        goto err_exercise3a;
    }
    Exercise3b = xTaskCreateStatic(vExercise3b, "Exercise3", TASK3B_STACK_SIZE,
                      NULL, mainGENERIC_PRIORITY+1, task3bStack, &Exercise3b_Buffer);
    if (!Exercise3b) {
        goto err_exercise3b;
    }
    if (xTaskCreate(vExercise4, "Exercise4", mainGENERIC_STACK_SIZE * 2, NULL,
                    mainGENERIC_PRIORITY, &Exercise4) != pdPASS) {
        goto err_exercise4;
    }
    if (xTaskCreate(vStatesHandler, "StatesHandler", 
                    mainGENERIC_STACK_SIZE * 2, NULL, configMAX_PRIORITIES-1,
                    &StatesHandler) != pdPASS) {
        goto err_statesHandler;
    }

    vTaskSuspend(Exercise2);
    vTaskSuspend(Exercise3a);
    vTaskSuspend(Exercise3b);
    vTaskSuspend(Exercise4);

    tumFUtilPrintTaskStateList();

    printf("Created state for Exercise 2 with ID %d. \n",
        initState(&state_param_ex2, NULL, exercise2enter, NULL, exercise2exit, NULL));
    printf("Created state for Exercise 3 with ID %d. \n",
        initState(&state_param_ex3, NULL, exercise3enter, NULL, exercise3exit, NULL));
    printf("Created state for Exercise 4 with ID %d. \n",
        initState(&state_param_ex4, NULL, exercise4enter, NULL, exercise4exit, NULL));

    /*//Example on how to delete states and what's happening with them
    state_parameters_t *test;
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

err_statesHandler:
    vTaskDelete(Exercise4);
err_exercise4:
    vTaskDelete(Exercise3b);
err_exercise3b:
    vTaskDelete(Exercise3a);
err_exercise3a:
    vTaskDelete(Exercise2);
err_exercise2:
    vTaskDelete(BufferSwap);
err_bufferswap:
    vSemaphoreDelete(exercise4Mutex);
err_ex4_sem:
    vSemaphoreDelete(exercise3bMutex);
err_ex3b_sem:
    vSemaphoreDelete(exercise3aMutex);
err_ex3a_sem:
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
