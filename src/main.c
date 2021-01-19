#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <SDL2/SDL_scancode.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "TUM_Draw.h"
#include "TUM_Event.h"
#include "TUM_Utils.h"
#include "TUM_FreeRTOS_Utils.h"
#include "TUM_Font.h"

#include "AsyncIO.h"

#define FRAMERATE       50

#define FONT1           "IBMPlexSans-Bold.ttf"
#define FONT2     		"IBMPlexSans-ThinItalic.ttf"
#define FONT3		    "IBMPlexSans-Medium.ttf"
#define FONT4			"IBMPlexSans-Thin.ttf"

#define TITLE_FONT_SIZE     40
#define PLAYER_FONT_SIZE    25
#define LEVEL_FONT_SIZE     45
#define NUMBERS_FONT_SIZE   35
#define HIGHSCORE_FONT_SIZE 40
#define NAMES_FONT_SIZE     20

#define LEVEL_Y             SCREEN_HEIGHT / 5
#define NUMBERS_SPACING     NUMBERS_FONT_SIZE * 1.5
#define HIGHSCORE_Y         SCREEN_HEIGHT * 3 / 5

#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)
#define mainGENERIC_STACK_SIZE ((unsigned short)2560)

static TaskHandle_t DemoTask2 = NULL;
static TaskHandle_t BufferSwapTask = NULL;

SemaphoreHandle_t DrawSignal  = NULL;
SemaphoreHandle_t ScreenLock = NULL;


void setFont(char *fontName, ssize_t fontSize)
{
    tumFontSelectFontFromName(fontName);
    tumFontSetSize(fontSize);
}

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


void vDemoTask2(void *pvParameters)
{
    
    unsigned int score = 1000;
    char my_string[100];        //array to temporarily store text
    int my_string_width = 0;    //temporarily store the width of the text
    font_handle_t cur_font;

    while (1) {
        if (DrawSignal) {   //Check if the Semaphore has been initialized
            cur_font = tumFontGetCurFontHandle();
            xSemaphoreTake(DrawSignal, portMAX_DELAY);

            //Take the ScreenLock to start drawing
            xSemaphoreTake(ScreenLock, portMAX_DELAY);
            // Clear screen
            tumDrawClear(White);

            setFont(FONT3, LEVEL_FONT_SIZE);
            sprintf(my_string, "Level");
            tumDrawText(my_string, SCREEN_WIDTH / 2, LEVEL_Y - LEVEL_FONT_SIZE, TUMBlue);

            setFont(FONT4, NUMBERS_FONT_SIZE);
            sprintf(my_string, "1 2 3");
            tumDrawText(my_string,
                            SCREEN_WIDTH/2 - 2*NUMBERS_SPACING,
                            LEVEL_Y + NUMBERS_FONT_SIZE / 2,
                            Black);

            setFont(FONT3, HIGHSCORE_FONT_SIZE);
            sprintf(my_string, "Highscores");
            tumDrawText(my_string, SCREEN_WIDTH / 2, HIGHSCORE_Y - HIGHSCORE_FONT_SIZE, TUMBlue);

            setFont(FONT4, NAMES_FONT_SIZE);
            sprintf(my_string, "Player");
            tumDrawText(my_string, SCREEN_WIDTH / 5, HIGHSCORE_Y + NAMES_FONT_SIZE, Black);

            //Switch back to the old font
            tumFontSelectFontFromHandle(cur_font);
            tumFontPutFontHandle(cur_font);

            //Drawing is done -> give the ScreenLock back
            xSemaphoreGive(ScreenLock);
        }
    }
}

int main(int argc, char *argv[])
{
    char *bin_folder_path = tumUtilGetBinFolderPath(argv[0]);

    printf("Initializing: ");

    if (tumDrawInit(bin_folder_path)) {
        PRINT_ERROR("Failed to initialize drawing");
        goto err_init_drawing;
    }

    if (tumEventInit()) {
        PRINT_ERROR("Failed to initialize events");
        goto err_init_events;
    }

    tumFontLoadFont(FONT1, DEFAULT_FONT_SIZE);
    tumFontLoadFont(FONT2, DEFAULT_FONT_SIZE);
    tumFontLoadFont(FONT3, DEFAULT_FONT_SIZE);
    tumFontLoadFont(FONT4, DEFAULT_FONT_SIZE);

    DrawSignal = xSemaphoreCreateBinary();
    ScreenLock = xSemaphoreCreateMutex();

    xTaskCreate(vDemoTask2, "DemoTask2", mainGENERIC_STACK_SIZE * 2, NULL,
                    mainGENERIC_PRIORITY, &DemoTask2);

    xTaskCreate(vSwapBuffers, "BufferSwapTask", mainGENERIC_STACK_SIZE * 2, NULL, 
                    configMAX_PRIORITIES, &BufferSwapTask);

    vTaskStartScheduler();

    return EXIT_SUCCESS;

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
