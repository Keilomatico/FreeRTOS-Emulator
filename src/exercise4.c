#include "exercise4.h"

void exercise4run(void *data)
{
    printf("Resuming task 4 \n");
    vTaskResume(Exercise4);
}

void exercise4exit(void *data)
{
    printf("Suspending task 4 \n");
    vTaskSuspend(Exercise4);
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
        vTaskDelay(10);
    }
}