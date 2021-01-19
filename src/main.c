//#include <math.h>
#include <stdio.h>
#include <stdlib.h>
//#include <time.h>
#include <inttypes.h>

//#include <SDL2/SDL_scancode.h>

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

#define BOLD_FONT           "IBMPlexSans-Bold.ttf"
#define THINITALIC_FONT     "IBMPlexSans-ThinItalic.ttf"
#define MEDIUM_FONT		    "IBMPlexSans-Medium.ttf"
#define THIN_FONT			"IBMPlexSans-Thin.ttf"

#define TITLE_FONT          BOLD_FONT
#define TITLE_FONT_SIZE     40
#define PLAYER_FONT         THINITALIC_FONT
#define PLAYER_FONT_SIZE    25

#define LEVEL_FONT          MEDIUM_FONT
#define LEVEL_FONT_SIZE     45
#define LEVEL_Y             SCREEN_HEIGHT / 5
#define NUMBERS_FONT        THIN_FONT
#define NUMBERS_FONT_SIZE   35
#define NUMBERS_SPACING     NUMBERS_FONT_SIZE * 1.5

#define HIGHSCORE_FONT      MEDIUM_FONT
#define HIGHSCORE_FONT_SIZE 40
#define HIGHSCORE_Y         SCREEN_HEIGHT * 3 / 5
#define NAMES_FONT          THIN_FONT
#define NAMES_FONT_SIZE     20


#define FONT1           "IBMPlexSans-Bold.ttf"
#define FONT2     		"IBMPlexSans-ThinItalic.ttf"
#define FONT3		    "IBMPlexSans-Medium.ttf"
#define FONT4			"IBMPlexSans-Thin.ttf"

#define SIZE1			10
#define SIZE2			18
#define SIZE3			20
#define SIZE4			25
#define SIZE5			30

#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)
#define mainGENERIC_STACK_SIZE ((unsigned short)2560)

#define Light_Gray   (unsigned int)(0xC0C0C0)

static TaskHandle_t DemoTask1 = NULL;
static TaskHandle_t DemoTask2 = NULL;
static TaskHandle_t ChangeState = NULL;
static TaskHandle_t BufferSwapTask = NULL;

SemaphoreHandle_t DrawSignal  = NULL;
SemaphoreHandle_t ScreenLock = NULL;

typedef struct buttons_buffer {
    unsigned char currentState[SDL_NUM_SCANCODES];
	unsigned char lastState[SDL_NUM_SCANCODES];
	TickType_t last[SDL_NUM_SCANCODES];
    SemaphoreHandle_t lock;
}buttons_buffer_t;

static buttons_buffer_t buttons = { 0 };

void xGetButtonInput(void)
{
    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
        xQueueReceive(buttonInputQueue, &buttons.currentState, 0);
        xSemaphoreGive(buttons.lock);
    }
}

int checkbutton(int keycode)
{
    TickType_t current_tick;
    int ret = 0;

    //Buttons is globally shared, so take the mutex first
    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
        current_tick = xTaskGetTickCount();
        //Check for rising edge
        if (buttons.currentState[keycode] > 0 && buttons.lastState[keycode] == 0) {
            //Check if the last and current edge are far enough apart
            if(current_tick - buttons.last[keycode] > 100)
                ret = 1;
            //In any case an edge was detected so update the timestamp
            buttons.last[keycode] = current_tick;
        }
        //Update lastState for the next call
        buttons.lastState[keycode] = buttons.currentState[keycode];
        xSemaphoreGive(buttons.lock);
    }
    return ret;
}

void setFont(char *fontName, ssize_t fontSize)
{
    tumFontSelectFontFromName(fontName);
    tumFontSetSize(fontSize);
}

void vChangeState(void *pvParameters)
{
    char state = 0;
    vTaskResume(DemoTask1);
    while(1)
    {
        xGetButtonInput(); // Update global input
        if(checkbutton(SDL_SCANCODE_RETURN) || checkbutton(SDL_SCANCODE_KP_ENTER)) {
            if(state == 0) {
                state = 1;
                vTaskSuspend(DemoTask1);
                vTaskResume(DemoTask2);
            }
            else {
                state = 0;
                vTaskSuspend(DemoTask2);
                vTaskResume(DemoTask1);
            }
        }
        vTaskDelay(10);
    }
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

/*void vDemoTask1(void *pvParameters)
{
    char my_string[100];
    font_handle_t cur_font;

    while (1) {
        xSemaphoreTake(DrawSignal, portMAX_DELAY);
        xSemaphoreTake(ScreenLock, portMAX_DELAY);

        tumDrawClear(White); // Clear screen

        cur_font = tumFontGetCurFontHandle();

        setFont(FONT1, (ssize_t)SIZE1);
        sprintf(my_string, "Hello From Screen 1");
        tumDrawText(my_string, 30, 30, TUMBlue);

        setFont(FONT2, (ssize_t)SIZE2);
        sprintf(my_string, "Test1b");
        tumDrawText(my_string, 150, 30, TUMBlue);

        tumFontSelectFontFromHandle(cur_font);
        tumFontPutFontHandle(cur_font);



        cur_font = tumFontGetCurFontHandle();

        setFont(FONT3, (ssize_t)SIZE2);
        sprintf(my_string, "Test2");
        tumDrawText(my_string, 30, 60, TUMBlue);
        
        setFont(FONT4, (ssize_t)SIZE3);
        sprintf(my_string, "Test3");
        tumDrawText(my_string, 30, 100, TUMBlue);

        setFont(FONT3, (ssize_t)SIZE4);
        sprintf(my_string, "Test4");
        tumDrawText(my_string, 30, 150, TUMBlue);

        setFont(FONT4, (ssize_t)SIZE5);
        sprintf(my_string, "Test5");
        tumDrawText(my_string, 30, 200, TUMBlue);

        

        //Switch back to the old font
        tumFontSelectFontFromHandle(cur_font);
        tumFontPutFontHandle(cur_font);

        xSemaphoreGive(ScreenLock);
    }
}*/

void vDemoTask1(void *pvParameters)
{
    //Stores the currently chosen mode (single- or two-player-mode); Init with single-player-mode
    enum mode {single, two} playmode = single;
    unsigned int color;

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

            //Select new font and size for the title
            setFont(TITLE_FONT, TITLE_FONT_SIZE);

            sprintf(my_string, "Welcome to Tetris");

            // Center the string
            if (!tumGetTextSize((char *)my_string, &my_string_width, NULL))
                tumDrawText(my_string,
                            SCREEN_WIDTH / 2 - my_string_width / 2,
                            SCREEN_HEIGHT / 3 - TITLE_FONT_SIZE / 2,
                            TUMBlue);

            //Select new font and size for the modes
            setFont(PLAYER_FONT, PLAYER_FONT_SIZE);

            if(playmode == single)
                color = Black;
            else
                color = Light_Gray;

            sprintf(my_string, "1 Player");
            if (!tumGetTextSize((char *)my_string, &my_string_width, NULL))
                tumDrawText(my_string,
                            SCREEN_WIDTH / 4 - my_string_width / 2,
                            SCREEN_HEIGHT * 2 / 3 - PLAYER_FONT_SIZE / 2,
                            color);

            if(playmode == two)
                color = Black;
            else
                color = Light_Gray;

            sprintf(my_string, "2 Player");
            if (!tumGetTextSize((char *)my_string, &my_string_width, NULL))
                tumDrawText(my_string,
                            SCREEN_WIDTH * 3 / 4 - my_string_width / 2,
                            SCREEN_HEIGHT * 2 / 3 - PLAYER_FONT_SIZE / 2,
                            color);

            //Switch back to the old font
            tumFontSelectFontFromHandle(cur_font);
            tumFontPutFontHandle(cur_font);

            //Drawing is done -> give the ScreenLock back
            xSemaphoreGive(ScreenLock);
        }
    }
}

void vDemoTask2(void *pvParameters)
{
    char level = 1;             //Chosen Level. Can be 1 to 10
    unsigned int color;
    
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

            //----------------------Print Level------------------------------------
            //Select new font and size for the "level"
            setFont(LEVEL_FONT, LEVEL_FONT_SIZE);
            sprintf(my_string, "Level");

            // Center the string
            if (!tumGetTextSize((char *)my_string, &my_string_width, NULL))
                //Centered in x and in y with the bottom line at LEVEL_Y
                tumDrawText(my_string,
                            SCREEN_WIDTH / 2 - my_string_width / 2,
                            LEVEL_Y - LEVEL_FONT_SIZE,
                            TUMBlue);

            setFont(NUMBERS_FONT, NUMBERS_FONT_SIZE);
            for(int i=1; i<=10; i++) {
                if(i == level)
                    color = Black;
                else
                    color = Light_Gray;

                sprintf(my_string, "%d", i);
                if (!tumGetTextSize((char *)my_string, &my_string_width, NULL))
                    //Print two rows, each with 5 numbers
                    //x: Center the whole block of numbers and space them with NUMBERS_FONT_SIZE * 1.5
                    //y: Start halb a font size below the LEVEL_Y and space first and second line with NUMBERS_SPACING
                    tumDrawText(my_string,
                                SCREEN_WIDTH/2 - 2*NUMBERS_SPACING + (i-1)%5 * NUMBERS_SPACING - my_string_width/2,
                                LEVEL_Y + (i-1)/5 * NUMBERS_SPACING + NUMBERS_FONT_SIZE / 2,
                                color);
            }

            //----------------------Print Highscore------------------------------------
            setFont(HIGHSCORE_FONT, HIGHSCORE_FONT_SIZE);
            sprintf(my_string, "Highscores");

            // Center the string
            if (!tumGetTextSize((char *)my_string, &my_string_width, NULL))
                //Centered in x and in y with the bottom line at LEVEL_Y
                tumDrawText(my_string,
                            SCREEN_WIDTH / 2 - my_string_width / 2,
                            HIGHSCORE_Y - HIGHSCORE_FONT_SIZE,
                            TUMBlue);

            setFont(NAMES_FONT, NAMES_FONT_SIZE);

            for(int i=0; i<3; i++)
            {
                sprintf(my_string, "Player");
                tumDrawText(my_string,
                                SCREEN_WIDTH / 5,
                                HIGHSCORE_Y + NAMES_FONT_SIZE + i * NAMES_FONT_SIZE * 1.5,
                                Black);
                sprintf(my_string, "%d", score);
                tumDrawText(my_string,
                                SCREEN_WIDTH * 2 / 3,
                                HIGHSCORE_Y + NAMES_FONT_SIZE  + i * NAMES_FONT_SIZE * 1.5,
                                Black);
            }

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

    buttons.lock = xSemaphoreCreateMutex();
    DrawSignal = xSemaphoreCreateBinary();
    ScreenLock = xSemaphoreCreateMutex();


    xTaskCreate(vDemoTask1, "DemoTask1", mainGENERIC_STACK_SIZE * 2, NULL,
                    mainGENERIC_PRIORITY, &DemoTask1);
    vTaskSuspend(DemoTask1);

    xTaskCreate(vDemoTask2, "DemoTask2", mainGENERIC_STACK_SIZE * 2, NULL,
                    mainGENERIC_PRIORITY, &DemoTask2);
    vTaskSuspend(DemoTask2);

    xTaskCreate(vSwapBuffers, "BufferSwapTask", mainGENERIC_STACK_SIZE * 2, NULL, 
                    configMAX_PRIORITIES, &BufferSwapTask);

    xTaskCreate(vChangeState, "StateChangeTask", mainGENERIC_STACK_SIZE * 2, NULL, 
                    5, &ChangeState);

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
