#include "exercise3.h"

void exercise3enter(void *data)
{
    printf("Resuming task 3 \n");
    xSemaphoreGive(exercise3aMutex);
    xSemaphoreGive(exercise3bMutex);
    vTaskResume(Exercise3a);
    vTaskResume(Exercise3b);
    xSemaphoreTake(ScreenLock, portMAX_DELAY);
    tumDrawClear(White); // Clear screen
    xSemaphoreGive(ScreenLock);
}

void exercise3exit(void *data)
{
    printf("Suspending task 3 \n");
    xSemaphoreTake(exercise3aMutex, portMAX_DELAY);
    xSemaphoreTake(exercise3bMutex, portMAX_DELAY);
    vTaskSuspend(Exercise3a);
    vTaskSuspend(Exercise3b);
}

void vExercise3a(void *pvParameters)
{    
    TickType_t prev_wake_time = xTaskGetTickCount();
    char state = 0;

    while (1) {
        xSemaphoreTake(exercise3aMutex, portMAX_DELAY);
        xSemaphoreTake(ScreenLock, portMAX_DELAY);
        if(state == 0){
            tumDrawCircle (SCREEN_WIDTH / 2 - CENTER_OFFSET,
                            SCREEN_HEIGHT / 2,
                            CIRCLE_RADIUS,
                            Red);
            state = 1;
        }
        else {
            tumDrawCircle (SCREEN_WIDTH / 2 - CENTER_OFFSET,
                            SCREEN_HEIGHT / 2,
                            CIRCLE_RADIUS,
                            White);
                state = 0;
        }
        xSemaphoreGive(ScreenLock);
        xSemaphoreGive(exercise3aMutex);
        
        vTaskDelayUntil(&prev_wake_time, TASK3A_INTERVAL);
    }
}

void vExercise3b(void *pvParameters)
{
    TickType_t prev_wake_time = xTaskGetTickCount();
    char state = 0;

    while (1) {
        xSemaphoreTake(exercise3bMutex, portMAX_DELAY);
        xSemaphoreTake(ScreenLock, portMAX_DELAY);
        if(state == 0){
            tumDrawCircle (SCREEN_WIDTH / 2 + CENTER_OFFSET,
                            SCREEN_HEIGHT / 2,
                            CIRCLE_RADIUS,
                            Blue);
            state = 1;
        }
        else {
            tumDrawCircle (SCREEN_WIDTH / 2 + CENTER_OFFSET,
                            SCREEN_HEIGHT / 2,
                            CIRCLE_RADIUS,
                            White);
                state = 0;
        }
        xSemaphoreGive(ScreenLock);
        xSemaphoreGive(exercise3bMutex);
        
        vTaskDelayUntil(&prev_wake_time, TASK3B_INTERVAL);
    }
}