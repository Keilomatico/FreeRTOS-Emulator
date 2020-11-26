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

#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)
#define mainGENERIC_STACK_SIZE ((unsigned short)2560)

#define ROTATION_RADIUS     100
#define DEFAULT_RADIUS      30
#define TRIANGLE_LENGTH     70
#define TRIANGLE_HEIGHT     60     
#define SQUARE_LENGTH       60
#define TEXT_OFFSET_Y       150

static TaskHandle_t DemoTask = NULL; //Init with NULL, so you can check if it has been initialized

typedef struct buttons_buffer {
    unsigned char buttons[SDL_NUM_SCANCODES];
    SemaphoreHandle_t lock;
} buttons_buffer_t;

static buttons_buffer_t buttons = { 0 };

void xGetButtonInput(void)
{
    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
        xQueueReceive(buttonInputQueue, &buttons.buttons, 0);
        xSemaphoreGive(buttons.lock);
    }
}

#define KEYCODE(CHAR) SDL_SCANCODE_##CHAR

void vDemoTask(void *pvParameters)
{
    /* ######## Initialization ############## */
    // structure to store my text
    static char my_string[100];
    static int my_string_width = 0;
    static coord_t mycoordinates[4];
    static float i=0;
    static int offset_x;
    static int offset_y;


    // Needed such that Gfx library knows which thread controlls drawing
    // Only one thread can call tumDrawUpdateScreen while and thread can call
    // the drawing functions to draw objects. This is a limitation of the SDL
    // backend.
    tumDrawBindThread();

    /* ############# Actual program ################ */
    while (1) {
        tumEventFetchEvents(FETCH_EVENT_NONBLOCK); // Query events backend for new events, ie. button presses
        xGetButtonInput(); // Update global input

        // `buttons` is a global shared variable and as such needs to be
        // guarded with a mutex, mutex must be obtained before accessing the
        // resource and given back when you're finished. If the mutex is not
        // given back then no other task can access the reseource.
        if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
            if (buttons.buttons[KEYCODE(
                                    Q)]) { // Equiv to SDL_SCANCODE_Q
                exit(EXIT_SUCCESS);
            }
            xSemaphoreGive(buttons.lock);
        }

        tumDrawClear(White); // Clear screen

        //Calculate offset for moving parts
        offset_x = (int) (ROTATION_RADIUS * cos(i));
        offset_y = (int) (ROTATION_RADIUS * sin(i));
        
        tumDrawArc 	(   SCREEN_WIDTH / 2 - offset_x, 
                        SCREEN_HEIGHT / 2 + offset_y,
		                DEFAULT_RADIUS,
		                0,
		                359,
	                    Purple);

        //Create the points for the triangle
        //Left point
        mycoordinates[0].x = SCREEN_WIDTH / 2 - TRIANGLE_LENGTH / 2;
        mycoordinates[0].y = SCREEN_HEIGHT / 2 + TRIANGLE_HEIGHT / 2;
        //Right point
        mycoordinates[1].x = SCREEN_WIDTH / 2 + TRIANGLE_LENGTH / 2;
        mycoordinates[1].y = SCREEN_HEIGHT / 2 + TRIANGLE_HEIGHT / 2;
        //Top point
        mycoordinates[2].x = SCREEN_WIDTH / 2;
        mycoordinates[2].y = SCREEN_HEIGHT / 2 - TRIANGLE_HEIGHT / 2;

        tumDrawPoly(mycoordinates, 3, Teal);  //Draw the triangle


        //Create the points for the square
        //Top left point
        mycoordinates[0].x = SCREEN_WIDTH / 2 - SQUARE_LENGTH / 2 + offset_x;
        mycoordinates[0].y = SCREEN_HEIGHT / 2 - SQUARE_LENGTH / 2 - offset_y;
        //Top right point
        mycoordinates[1].x = SCREEN_WIDTH / 2 + SQUARE_LENGTH / 2 + offset_x;
        mycoordinates[1].y = SCREEN_HEIGHT / 2 - SQUARE_LENGTH / 2 - offset_y;
        //Bottom right point
        mycoordinates[2].x = SCREEN_WIDTH / 2 + SQUARE_LENGTH / 2 + offset_x;
        mycoordinates[2].y = SCREEN_HEIGHT / 2 + SQUARE_LENGTH / 2 - offset_y;
        //Bottom left point
        mycoordinates[3].x = SCREEN_WIDTH / 2 - SQUARE_LENGTH / 2 + offset_x;
        mycoordinates[3].y = SCREEN_HEIGHT / 2 + SQUARE_LENGTH / 2 - offset_y;

        tumDrawPoly(mycoordinates, 4, Fuchsia);  //Draw the sqare

        // Format the string into the char array
        sprintf(my_string, "Hello World");
        // Get the width of the string on the screen so we can center it
        // Returns 0 if width was successfully obtained
        if (!tumGetTextSize((char *)my_string, &my_string_width, NULL))
            tumDrawText(my_string,
                        SCREEN_WIDTH / 2 - my_string_width / 2,
                        SCREEN_HEIGHT / 2 - DEFAULT_FONT_SIZE / 2 + TEXT_OFFSET_Y,
                        TUMBlue);

        tumDrawUpdateScreen(); // Refresh the screen to draw string

        if(i >= 6.283)
            i=0;
        else
            i += 0.02;
        

        vTaskDelay((TickType_t)1);
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

    /* Here You can start modifying  */
    if (xTaskCreate(vDemoTask, "DemoTask", mainGENERIC_STACK_SIZE * 2, NULL,
                    mainGENERIC_PRIORITY, &DemoTask) != pdPASS) {
        goto err_demotask;
    }

    vTaskStartScheduler();

    return EXIT_SUCCESS;

err_demotask:
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
