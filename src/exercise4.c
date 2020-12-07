#include "exercise4.h"

void exercise4enter(void *data)
{
    printf("Resuming task 4 \n");
    xSemaphoreGive(exercise4Mutex);
    vTaskResume(Exercise4draw);
}

void exercise4exit(void *data)
{
    printf("Suspending task 4 \n");
    xSemaphoreTake(exercise4Mutex, portMAX_DELAY);
    vTaskSuspend(Exercise4draw);
}

void vExercise4draw(void *pvParameters)
{    
    char my_string[TICK_NUM][EX4_ARRAY_LENGTH] = { 0 };
    int offset;
    TickType_t start_tick;
    int tickCounter;
    int lastCounterVal;
    int temp;
    char firstCall = 1;

    while (1) {
        xSemaphoreTake(exercise4Mutex, portMAX_DELAY);
        if(firstCall) {
            firstCall = 0;
            start_tick = xTaskGetTickCount();
            tickCounter = 0;
            lastCounterVal = 0;
            temp = 0;
            
            printf("Exercise4draw started. Resuming tasks \n");
            vTaskResume(Exercise4task1);
            vTaskResume(Exercise4task2);
            vTaskResume(Exercise4task3);
            vTaskResume(Exercise4task4);
            sprintf(my_string[tickCounter], "Tick %d: ", tickCounter+1);
            while(tickCounter < TICK_NUM)
            {
                //Add new state
                //This has do be done here, so that the last notification that comes
                //in after the 15 ticks doesn't get appended. As temp is initialized
                //with 0 the first call is also no problem.
                if(lastCounterVal != tickCounter) {
                    sprintf(my_string[tickCounter], "Tick %d: ", tickCounter+1);
                    lastCounterVal = tickCounter;
                }
                if(temp & BIT_TASK1_RUNNING)
                    appendToStr(my_string[tickCounter], '1');
                else if(temp & BIT_TASK2_RUNNING)
                    appendToStr(my_string[tickCounter], '2');
                else if(temp & BIT_TASK3_RUNNING)
                    appendToStr(my_string[tickCounter], '3');
                else if(temp & BIT_TASK4_RUNNING)
                    appendToStr(my_string[tickCounter], '4');

                temp = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
                tickCounter = xTaskGetTickCount()-start_tick;
            }

            //vTaskSuspend(BufferSwap);
            //vTaskSuspend(StatesHandler);
            printf("Finished. Suspending tasks \n");
            //vTaskResume(StatesHandler);
            //vTaskResume(BufferSwap);
            vTaskSuspend(Exercise4task1);
            vTaskSuspend(Exercise4task2);
            vTaskSuspend(Exercise4task3);
            vTaskSuspend(Exercise4task4);
        }
        if (DrawSignal) {
            xSemaphoreTake(DrawSignal, portMAX_DELAY);
            xSemaphoreTake(ScreenLock, portMAX_DELAY);
            
            tumDrawClear(White); // Clear screen

            offset = 0;
            for(int i=0; i<TICK_NUM; i++)
            {
                tumDrawText(my_string[i], POS_X, POS_Y+offset, TUMBlue);
                offset += DEFAULT_FONT_SIZE;
            }
                
            xSemaphoreGive(ScreenLock);
        }
        
        xSemaphoreGive(exercise4Mutex);
    }
}

void vExercise4task1(void *pvParameters)
{
    TickType_t prev_wake_time = xTaskGetTickCount();

    while(1)
    {
        xTaskNotify(Exercise4draw, BIT_TASK1_RUNNING, eSetBits);
        vTaskDelayUntil(&prev_wake_time, 1);
    }
}

void vExercise4task2(void *pvParameters)
{
    TickType_t prev_wake_time = xTaskGetTickCount();

    while(1)
    {
        xTaskNotify(Exercise4draw, BIT_TASK2_RUNNING, eSetBits);
        xSemaphoreGive(task3Notify);
        vTaskDelayUntil(&prev_wake_time, 2);
    }
}

void vExercise4task3(void *pvParameters)
{
    while(1)
    {
        if(xSemaphoreTake(task3Notify, portMAX_DELAY))
            xTaskNotify(Exercise4draw, BIT_TASK3_RUNNING, eSetBits);
    }
}

void vExercise4task4(void *pvParameters)
{
    TickType_t prev_wake_time = xTaskGetTickCount();

    while(1)
    {
        xTaskNotify(Exercise4draw, BIT_TASK4_RUNNING, eSetBits);
        vTaskDelayUntil(&prev_wake_time, 4);
    }
}