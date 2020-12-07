#include "exercise3.h"

void exercise3enter(void *data)
{
    unsigned int temp = 0;
    printf("Resuming tasks of state 3 \n");
    xSemaphoreGive(exercise3Mutex);
    xTaskNotify(Exercise3count, BIT_UPDATE_TIME, eSetBits);
    vTaskResume(Exercise3draw);
    vTaskResume(Exercise3circle1);
    vTaskResume(Exercise3circle2);
    vTaskResume(Exercise3button1);
    vTaskResume(Exercise3button2);
    vTaskResume(Exercise3count);
    xTimerStart(Exercise3timer, portMAX_DELAY);

    //Send a zero to the queues if they are empty
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
    xTimerStop(Exercise3timer, portMAX_DELAY);
}

void vExercise3draw(void *pvParameters)
{
    char my_string[100];            //array to temporarily store text
    unsigned int input = 0;         //temporarily used to store the notification value 
    
    /**
     * Value storing the state of the task. In this case the state of the 2 circles.
     * This is a bit complicated:
     * As a task cannot clear a notification value, there has to be another way
     * of informing that the state of xy should be 0 now.
     * So there are two bits for each state: One for setting it and one for clearing it.
     * But there is another problem: In order to have bits cleared in the notification 
     * value, the clearOnExit flag in ulTaskNotifyTake is set to one. However, this created
     * the need for a variable which stores the information persistantly -> taskState.
     * How does it work?
     * Each time a notification is coming in, the task checks if one of the bits should
     * be set or cleared. So internally in this task, only the _ON bit is beeing used.
    */
    static unsigned int taskState = 0;

    static char counterEnab = 1;

    while (1) {
        if (DrawSignal) {
            xSemaphoreTake(exercise3Mutex, portMAX_DELAY);
            xSemaphoreTake(DrawSignal, portMAX_DELAY);
            xGetButtonInput(); // Update global input

            //Inform the other tasks of button changes
            if (checkbutton(KEYCODE(N))) {
                xSemaphoreGive(button1Notify);
            }
            if (checkbutton(KEYCODE(M))) {
                xTaskNotify(Exercise3button2, BIT_BUTTON2, eSetBits);
            }
            //Enable or disable the counter
            if (checkbutton(KEYCODE(X))) {
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

            //Update the task state (see above for a detailed explanation)
            input = ulTaskNotifyTake(pdTRUE, 0);
            if(input != 0) {
                if(input & BIT_CIRCLE1_ON)
                    taskState |= BIT_CIRCLE1_ON;
                if(input & BIT_CIRCLE1_OFF)
                    taskState &= ~BIT_CIRCLE1_ON;
                if(input & BIT_CIRCLE2_ON)
                    taskState |= BIT_CIRCLE2_ON;
                if(input & BIT_CIRCLE2_OFF)
                    taskState &= ~BIT_CIRCLE2_ON;
            }
            //Draw the circles if the taskState says so
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
            
            //Print the number of button presses
            if(xQueuePeek(button1Num, &input, 0)) {
                sprintf(my_string, "Button N has been presses %d times.", input); 
                tumDrawText(my_string, BUTTON_TEXT_X, BUTTON_TEXT1_Y, Black);
            }
            if(xQueuePeek(button2Num, &input, 0)) {
                sprintf(my_string, "Button M has been presses %d times.", input); 
                tumDrawText(my_string, BUTTON_TEXT_X, BUTTON_TEXT2_Y, Black);
            }
            //Print the counter
            if(xQueuePeek(counterVal, &input, 0)) {
                sprintf(my_string, "Counter (press X to start and stop): %d", input); 
                tumDrawText(my_string, BUTTON_TEXT_X, BUTTON_TEXT3_Y, Black);
            }

            xSemaphoreGive(ScreenLock);
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

    while(1)
    {
        if(xSemaphoreTake(button1Notify, 0) == pdTRUE) {
            counter++;
            //xQueueOverwrite is used, because the queue should be full at all times
            xQueueOverwrite(button1Num, &counter);
        }

        //Use task notification to reset the counter
        if(ulTaskNotifyTake(pdTRUE, 0) & BIT_RESET_COUNTER) {
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

    /**
     * Note: In this task the notification are just used
     * to trigger events one single time. Therefore, there
     * is no need for persistency and the input can be simply
     * checked for the flags.
     */

    while(1)
    {
        input = ulTaskNotifyTake(pdTRUE, 0);
        
        if(input & BIT_BUTTON2) {
            counter++;
            xQueueOverwrite(button2Num, &counter);
        }
        if(input & BIT_RESET_COUNTER) {
            counter = 0;
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
        //This method for updating the prev_wake_time is needed, because the
        //task can be suspended and then the prev_wake_time would be outdated.
        if(ulTaskNotifyTake(pdTRUE, 0) & BIT_UPDATE_TIME)
            prev_wake_time = xTaskGetTickCount();
        counter++;
        xQueueOverwrite(counterVal, &counter);
        vTaskDelayUntil(&prev_wake_time, COUNTER_INTERVAL);
    }
}

void vExercise3timerCallback(TimerHandle_t xTimer)
{
    xTaskNotify(Exercise3button1, BIT_RESET_COUNTER, eSetBits);
    xTaskNotify(Exercise3button2, BIT_RESET_COUNTER, eSetBits);
}