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
#define MYCIRCLE_RADIUS     30
#define TRIANGLE_WIDTH      70
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

/**
 * @brief Calculates the coordinates for a square
 *
 * @param coordinates Points array to return the calculated points (must be at least of length 4)
 * @param x X coordinate of the center of the square
 * @param y Y coordinate of the center of the square
 * @param length Length of each side of the square in pixels
 * @return 0 on success
 */
int getSquareCorrdinates(coord_t *coordinates, int x, int y, int length)
{
    //Top left point
    coordinates[0].x = x - length / 2;
    coordinates[0].y = y - length / 2;
    //Top right point
    coordinates[1].x = x + length / 2;
    coordinates[1].y = y - length / 2;
    //Bottom right point
    coordinates[2].x = x + length / 2;
    coordinates[2].y = y + length / 2;
    //Bottom left point
    coordinates[3].x = x - length / 2;
    coordinates[3].y = y + length / 2;

    return 0;
}

/**
 * @brief Calculates the coordinates for a triangle
 *
 * @param coordinates Points array to return the calculated points (must be at least of length 3)
 * @param x X coordinate of the center of the square
 * @param y Y coordinate of the center of the square
 * @param width Length of the base side in pixels
 * @param height Height of the triangle in pixels
 * @return 0 on success
 */
int getTriangleCoordinates(coord_t *coordinates, int x, int y, int width, int height)
{
    //Left point
    coordinates[0].x = x - width / 2;
    coordinates[0].y = y + height / 2;
    //Right point
    coordinates[1].x = x + width / 2;
    coordinates[1].y = y + height / 2;
    //Top point
    coordinates[2].x = x;
    coordinates[2].y = y - height / 2;

    return 0;
}

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

        //Calculate offset for rotating parts
        offset_x = (int) (ROTATION_RADIUS * cos(i));
        offset_y = (int) (ROTATION_RADIUS * sin(i));
 
        //Draw the Circle 
        //tumDrawArc is unsed instead of tumDrawCircle to create just the outline and not a filled circle
        tumDrawArc(  SCREEN_WIDTH / 2 - offset_x, 
                        SCREEN_HEIGHT / 2 + offset_y,
		                MYCIRCLE_RADIUS,
                        0,
                        359,
	                    Purple);

        //Create the points for the triangle
        getTriangleCoordinates(mycoordinates, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, TRIANGLE_WIDTH, TRIANGLE_HEIGHT);

        tumDrawPoly(mycoordinates, 3, Teal);  //Draw the triangle


        //Create the points for the square
        getSquareCorrdinates(mycoordinates, SCREEN_WIDTH / 2 + offset_x, SCREEN_HEIGHT / 2 - offset_y, SQUARE_LENGTH);

        tumDrawPoly(mycoordinates, 4, Fuchsia);  //Draw the sqare

        // Format the string into the char array
        sprintf(my_string, "Hello ESPL");
        // Get the width of the string on the screen so we can center it
        // Returns 0 if width was successfully obtained
        if (!tumGetTextSize((char *)my_string, &my_string_width, NULL))
            tumDrawText(my_string,
                        SCREEN_WIDTH / 2 - my_string_width / 2,
                        SCREEN_HEIGHT / 2 - DEFAULT_FONT_SIZE / 2 + TEXT_OFFSET_Y,
                        TUMBlue);
        
        
        sprintf(my_string, "This is exercise 2.1");
        // Get the width of the string on the screen so it can be centered
        if (!tumGetTextSize((char *)my_string, &my_string_width, NULL))
        {
            //Calculate the offset for the text (offset_x is reused here)
            offset_x = (int) (offset_x * (SCREEN_WIDTH / 2 - my_string_width / 2) / ROTATION_RADIUS); 
            tumDrawText(my_string,
                        SCREEN_WIDTH / 2 - my_string_width / 2 + offset_x,
                        SCREEN_HEIGHT / 2 - DEFAULT_FONT_SIZE / 2 - TEXT_OFFSET_Y,
                        TUMBlue);
        }

        tumDrawUpdateScreen(); // Refresh the screen to draw string

        if(i >= 2*M_PI)
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
