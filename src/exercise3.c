#include "exercise3.h"

static void takeEx3Mutex(void)
{
    for(int i=0; i<EX3_TASK_NUM; i++)
        xSemaphoreTake(exercise3Mutex[i], portMAX_DELAY);
}

static void giveEx3Mutex(void)
{
    for(int i=0; i<EX3_TASK_NUM; i++)
        xSemaphoreGive(exercise3Mutex[i]);
}

void exercise3enter(void *data)
{
    printf("Resuming tasks of state 3 \n");
    xSemaphoreTake(ScreenLock, portMAX_DELAY);
    tumDrawClear(White); // Clear screen
    xSemaphoreGive(ScreenLock);
    giveEx3Mutex();
    vTaskResume(Exercise3a);
    vTaskResume(Exercise3b);
}

void exercise3exit(void *data)
{
    printf("Suspending tasks of state 3 \n");
    takeEx3Mutex();
    vTaskSuspend(Exercise3a);
    vTaskSuspend(Exercise3b);
}

void vExercise3a(void *pvParameters)
{    
    TickType_t prev_wake_time = xTaskGetTickCount();
    char state = 0;

    while (1) {
        xSemaphoreTake(exercise3Mutex[0], portMAX_DELAY);
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
        xSemaphoreGive(exercise3Mutex[0]);

        tumFUtilPrintTaskStateList();
        
        vTaskDelayUntil(&prev_wake_time, TASK3A_INTERVAL);
    }
}

void vExercise3b(void *pvParameters)
{
    TickType_t prev_wake_time = xTaskGetTickCount();
    char state = 0;

    while (1) {
        xSemaphoreTake(exercise3Mutex[1], portMAX_DELAY);
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
        xSemaphoreGive(exercise3Mutex[1]);
        
        vTaskDelayUntil(&prev_wake_time, TASK3B_INTERVAL);
    }
}