#include "exercise3.h"

void exercise3run(void *data)
{
    printf("Resuming task 3 \n");
    vTaskResume(Exercise3);
}

void exercise3exit(void *data)
{
    printf("Suspending task 3 \n");
    vTaskSuspend(Exercise3);
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
        vTaskDelay(10);
    }
}