#include "global.h"
#include "miscFunc.h"
#include "statemachine.h"
#include "exercise2.h"
#include "exercise3.h"
#include "exercise4.h"

#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)
#define mainGENERIC_STACK_SIZE ((unsigned short)2560)

TaskHandle_t Exercise2 = NULL; //Init with NULL, so you can check if it has been initialized
TaskHandle_t Exercise3 = NULL;
TaskHandle_t Exercise4 = NULL;
TaskHandle_t BufferSwap = NULL;
TaskHandle_t StatesHandler = NULL;

SemaphoreHandle_t DrawSignal  = NULL;
SemaphoreHandle_t ScreenLock = NULL;
SemaphoreHandle_t exercise2Sem = NULL;
buttons_buffer_t buttons = { 0 };

//Do I need to lock these?
static state_parameters_t state_param_ex2 = { 0 };
static state_parameters_t state_param_ex3 = { 0 };
static state_parameters_t state_param_ex4 = { 0 };

void vSwapBuffers(void *pvParameters)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    const TickType_t frameratePeriod = 20;

    tumDrawBindThread(); // Setup Rendering handle with correct GL context

    while (1) {
        if (xSemaphoreTake(ScreenLock, portMAX_DELAY) == pdTRUE) {
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
    /* Just leave all this stuff in here */
    char *bin_folder_path = tumUtilGetBinFolderPath(argv[0]); //Required for images

    printf("Initializing: ");

    /* Initialization (mostly of the mutexes) */
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
    exercise2Sem = xSemaphoreCreateBinary();
    if (!exercise2Sem) {
        PRINT_ERROR("Failed to create exercise2 semaphore");
        goto err_ex2_sem;
    }
    
    if (xTaskCreate(vSwapBuffers, "BufferSwapTask",
                    mainGENERIC_STACK_SIZE * 2, NULL, configMAX_PRIORITIES,
                    BufferSwap) != pdPASS) {
        goto err_bufferswap;
    }
    if (xTaskCreate(vExercise2, "Exercise2", mainGENERIC_STACK_SIZE * 2, NULL,
                    mainGENERIC_PRIORITY, &Exercise2) != pdPASS) {
        goto err_exercise2;
    }
    if (xTaskCreate(vExercise3, "Exercise3", mainGENERIC_STACK_SIZE * 2, NULL,
                    mainGENERIC_PRIORITY, &Exercise3) != pdPASS) {
        goto err_exercise3;
    }
    if (xTaskCreate(vExercise4, "Exercise4", mainGENERIC_STACK_SIZE * 2, NULL,
                    mainGENERIC_PRIORITY, &Exercise4) != pdPASS) {
        goto err_exercise4;
    }
    if (xTaskCreate(vStatesHandler, "StatesHandler", mainGENERIC_STACK_SIZE * 2, NULL,
                    mainGENERIC_PRIORITY+1, &StatesHandler) != pdPASS) {
        goto err_statesHandler;
    }

    //vTaskSuspend(Exercise2);
    vTaskSuspend(Exercise3);
    vTaskSuspend(Exercise4);

    printf("Created state for Exercise 2 with ID %d. \n",
        initState(&state_param_ex2, NULL, exercise2run, NULL, exercise2exit, NULL));
    printf("Created state for Exercise 3 with ID %d. \n",
        initState(&state_param_ex3, NULL, exercise3run, NULL, exercise3exit, NULL));
    printf("Created state for Exercise 4 with ID %d. \n",
        initState(&state_param_ex4, NULL, exercise4run, NULL, exercise4exit, NULL));


    vTaskStartScheduler();

    return EXIT_SUCCESS;

err_statesHandler:
    vTaskDelete(Exercise4);
err_exercise4:
    vTaskDelete(Exercise3);
err_exercise3:
    vTaskDelete(Exercise2);
err_exercise2:
    vTaskDelete(BufferSwap);
err_bufferswap:
    vSemaphoreDelete(exercise2Sem);
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
