#include "exercise3.h"

void exercise3enter(void *data)
{
    unsigned int temp = 0;
    printf("Resuming tasks of state 3 \n");
    xSemaphoreGive(exercise3Mutex);
    xTaskNotify(Exercise3count, BIT_RESET_COUNTER, eSetBits);
    vTaskResume(Exercise3draw);
    vTaskResume(Exercise3circle1);
    vTaskResume(Exercise3circle2);
    vTaskResume(Exercise3button1);
    vTaskResume(Exercise3button2);
    vTaskResume(Exercise3count);
    vTaskResume(Exercise3timer);

    xQueueSend(button1Num, &temp, 0);
    xQueueSend(button2Num, &temp, 0);
    xQueueSend(counterVal, &temp, 0);
}

void exercise3exit(void *data)
{
    printf("Suspending tasks of state 3 \n");
    xSemaphoreTake(exercise3Mutex, portMAX_DELAY);
    vTaskSuspend(Exercise3draw);
    vTaskSuspend(Exercise3circle1);
    vTaskSuspend(Exercise3circle2);
    vTaskSuspend(Exercise3button1);
    vTaskSuspend(Exercise3button2);
    vTaskSuspend(Exercise3count);
    vTaskSuspend(Exercise3timer);
}

void vExercise3draw(void *pvParameters)
{
    char my_string[100];
    unsigned int input = 0;
    unsigned int taskState = 0;

    static char counterEnab = 1;

    TickType_t last_changeN = xTaskGetTickCount();
    TickType_t last_changeM = xTaskGetTickCount();
    TickType_t last_changeX = xTaskGetTickCount();

    while (1) {
        if (DrawSignal) {
            xSemaphoreTake(exercise3Mutex, portMAX_DELAY);
            if (xSemaphoreTake(DrawSignal, portMAX_DELAY) == pdTRUE) {
                xGetButtonInput(); // Update global input

                if (checkbutton(&last_changeN, KEYCODE(N))) {
                    xSemaphoreGive(button1Notify);
                }
                if (checkbutton(&last_changeM, KEYCODE(M))) {
                    xTaskNotify(Exercise3button2, BIT_BUTTON2, eSetBits);
                }
                if (checkbutton(&last_changeX, KEYCODE(X))) {
                    counterEnab = !counterEnab;
                    if(counterEnab) {
                        xTaskNotify(Exercise3count, BIT_RESET_COUNTER, eSetBits);
                        vTaskResume(Exercise3count);
                    }
                    else
                        vTaskSuspend(Exercise3count);
                }
                
                tumDrawClear(White); // Clear screen
                vDrawFPS();

                input = ulTaskNotifyTake(pdTRUE, 0);
                if(input != 0)
                    taskState = input;
                if(taskState & BIT_CIRCLE1_ON)
                    tumDrawCircle (SCREEN_WIDTH / 2 - CENTER_OFFSET,
                                    SCREEN_HEIGHT / 2,
                                    CIRCLE_RADIUS,
                                    Red);
                if(taskState & BIT_CIRCLE2_ON)
                    tumDrawCircle (SCREEN_WIDTH / 2 + CENTER_OFFSET,
                                    SCREEN_HEIGHT / 2,
                                    CIRCLE_RADIUS,
                                    Blue);
                
                if(xQueuePeek(button1Num, &input, 0)) {
                    sprintf(my_string, "Button N has been presses %d times.", input); 
                    tumDrawText(my_string, BUTTON_TEXT_X, BUTTON_TEXT1_Y, Black);
                }

                if(xQueuePeek(button2Num, &input, 0)) {
                    sprintf(my_string, "Button M has been presses %d times.", input); 
                    tumDrawText(my_string, BUTTON_TEXT_X, BUTTON_TEXT2_Y, Black);
                }

                if(xQueuePeek(counterVal, &input, 0)) {
                    sprintf(my_string, "Counter (press X to start and stop): %d", input); 
                    tumDrawText(my_string, BUTTON_TEXT_X, BUTTON_TEXT3_Y, Black);
                }

                xSemaphoreGive(ScreenLock);
            }
            xSemaphoreGive(exercise3Mutex);
        }
     }
}

void vExercise3circle1(void *pvParameters)
{    
    TickType_t prev_wake_time = xTaskGetTickCount();
    char state = 1;

    while (1) {
        if(state){
            xTaskNotify(Exercise3draw, BIT_CIRCLE1_ON, eSetBits);
            state = 0;
        }
        else {
            xTaskNotify(Exercise3draw, BIT_CIRCLE1_OFF, eSetBits);
            state = 1;
        }
        vTaskDelayUntil(&prev_wake_time, TASK3A_INTERVAL);
    }
}

void vExercise3circle2(void *pvParameters)
{
    TickType_t prev_wake_time = xTaskGetTickCount();
    char state = 1;

    while (1) {
        if(state){
            xTaskNotify(Exercise3draw, BIT_CIRCLE2_ON, eSetBits);
            state = 0;
        }
        else {
            xTaskNotify(Exercise3draw, BIT_CIRCLE2_OFF, eSetBits);
            state = 1;
        }
        vTaskDelayUntil(&prev_wake_time, TASK3B_INTERVAL);
    }
}

void vExercise3button1(void *pvParameters)
{
    unsigned int counter = 0;
    unsigned int input = 0;
    unsigned int taskState = 0;

    while(1)
    {
        input = ulTaskNotifyTake(pdTRUE, 0);
        if(input != 0)
            taskState = input;

        if(xSemaphoreTake(button1Notify, 0) == pdTRUE) {
            counter++;
            xQueueOverwrite(button1Num, &counter);
        }

        if(taskState & BIT_RESET_COUNTER) {
            taskState &= ~BIT_RESET_COUNTER;
            counter = 0;
            xQueueOverwrite(button1Num, &counter);
        }

        vTaskDelay(DEFAULT_TASK_DELAY);
    }
}

void vExercise3button2(void *pvParameters)
{
    unsigned int counter = 0;
    unsigned int input = 0;
    unsigned int taskState = 0;

    while(1)
    {
        input = ulTaskNotifyTake(pdTRUE, 0);
        if(input != 0)
            taskState = input;
        
        if(taskState & BIT_BUTTON2) {
            counter++;
            taskState &= ~BIT_BUTTON2;
            xQueueOverwrite(button2Num, &counter);
        }
        if(taskState & BIT_RESET_COUNTER) {
            counter = 0;
            taskState &= ~BIT_RESET_COUNTER;
            xQueueOverwrite(button2Num, &counter);
        }

        vTaskDelay(DEFAULT_TASK_DELAY);
    }
}

void vExercise3count(void *pvParameters)
{
    TickType_t prev_wake_time = xTaskGetTickCount();
    unsigned int counter = 0;

    while(1)
    {
        if(ulTaskNotifyTake(pdTRUE, 0) & BIT_UPDATE_TIME)
            prev_wake_time = xTaskGetTickCount();
        counter++;
        xQueueOverwrite(counterVal, &counter);
        vTaskDelayUntil(&prev_wake_time, COUNTER_INTERVAL);
    }
}

void vExercise3timer(void *pvParameters)
{
    while(1)
    {
        xTaskNotify(Exercise3button1, BIT_RESET_COUNTER, eSetBits);
        xTaskNotify(Exercise3button2, BIT_RESET_COUNTER, eSetBits);
        vTaskDelay(5000);
    }
}