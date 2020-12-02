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

#include "global.h"
#include "miscFunc.h"
#include "exercise2.h"

#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)
#define mainGENERIC_STACK_SIZE ((unsigned short)2560)

static TaskHandle_t Exercise2 = NULL; //Init with NULL, so you can check if it has been initialized
static TaskHandle_t Exercise3 = NULL;
static TaskHandle_t Exercise4 = NULL;
static TaskHandle_t BufferSwap = NULL;

SemaphoreHandle_t DrawSignal  = NULL;
SemaphoreHandle_t ScreenLock = NULL;
struct buttons_buffer buttons = { 0 };

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

void vExercise3(void *pvParameters)
{    
    static char my_string[100]; // structure to store my text
    static int my_string_width = 0;

    while (1) {
        if (DrawSignal) {
            if (xSemaphoreTake(DrawSignal, portMAX_DELAY) == pdTRUE) {
                xSemaphoreTake(ScreenLock, portMAX_DELAY);
                
                tumDrawClear(White); // Clear screen

                // Format the string into the char array
                sprintf(my_string, "This is exercise 3");
                // Get the width of the string on the screen so we can center it
                // Returns 0 if width was successfully obtained
                if (!tumGetTextSize((char *)my_string, &my_string_width, NULL))
                    tumDrawText(my_string,
                                SCREEN_WIDTH / 2 - my_string_width / 2,
                                SCREEN_HEIGHT / 2 - DEFAULT_FONT_SIZE / 2,
                                TUMBlue);
                xSemaphoreGive(ScreenLock);
            }
        }
    }
}

void vExercise4(void *pvParameters)
{    
    static char my_string[100]; // structure to store my text
    static int my_string_width = 0;

    while (1) {
        if (DrawSignal) {
            if (xSemaphoreTake(DrawSignal, portMAX_DELAY) == pdTRUE) {
                xSemaphoreTake(ScreenLock, portMAX_DELAY);
                
                tumDrawClear(White); // Clear screen

                // Format the string into the char array
                sprintf(my_string, "This is exercise 4");
                // Get the width of the string on the screen so we can center it
                // Returns 0 if width was successfully obtained
                if (!tumGetTextSize((char *)my_string, &my_string_width, NULL))
                    tumDrawText(my_string,
                                SCREEN_WIDTH / 2 - my_string_width / 2,
                                SCREEN_HEIGHT / 2 - DEFAULT_FONT_SIZE / 2,
                                TUMBlue);
                xSemaphoreGive(ScreenLock);
            }
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

    //vTaskSuspend(vExercise2);
    vTaskSuspend(Exercise3);
    vTaskSuspend(Exercise4);

    vTaskStartScheduler();

    return EXIT_SUCCESS;

err_exercise4:
    vTaskDelete(Exercise3);
err_exercise3:
    vTaskDelete(Exercise2);
err_exercise2:
    vTaskDelete(BufferSwap);
err_bufferswap:
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
