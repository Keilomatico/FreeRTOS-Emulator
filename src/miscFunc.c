#include "miscFunc.h"

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
            if(current_tick - buttons.lastEdge[keycode] > DEBOUNCE_DELAY)
                ret = 1;
            //In any case an edge was detected so update the timestamp
            buttons.lastEdge[keycode] = current_tick;
        }
        //Update lastState for the next call
        buttons.lastState[keycode] = buttons.currentState[keycode];
        xSemaphoreGive(buttons.lock);
    }
    return ret;
}

int getRectCorrdinates(coord_t *coordinates, int x, int y, int x_length, int y_length)
{
    //Top left point
    coordinates[0].x = x - x_length / 2;
    coordinates[0].y = y - y_length / 2;
    //Top right point
    coordinates[1].x = x + x_length / 2;
    coordinates[1].y = y - y_length / 2;
    //Bottom right point
    coordinates[2].x = x + x_length / 2;
    coordinates[2].y = y + y_length / 2;
    //Bottom left point
    coordinates[3].x = x - x_length / 2;
    coordinates[3].y = y + y_length / 2;

    return 0;
}

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

void vDrawFPS(void)
{
    static unsigned int periods[FPS_AVERAGE_COUNT] = { 0 };
    static unsigned int periods_total = 0;
    static unsigned int index = 0;
    static unsigned int average_count = 0;
    static TickType_t xLastWakeTime = 0, prevWakeTime = 0;
    static char str[10] = { 0 };
    static int text_width;
    int fps = 0;
    font_handle_t cur_font = tumFontGetCurFontHandle();

    if (average_count < FPS_AVERAGE_COUNT) {
        average_count++;
    }
    else {
        periods_total -= periods[index];
    }

    xLastWakeTime = xTaskGetTickCount();

    if (prevWakeTime != xLastWakeTime) {
        periods[index] =
            configTICK_RATE_HZ / (xLastWakeTime - prevWakeTime);
        prevWakeTime = xLastWakeTime;
    }
    else {
        periods[index] = 0;
    }

    periods_total += periods[index];

    if (index == (FPS_AVERAGE_COUNT - 1)) {
        index = 0;
    }
    else {
        index++;
    }

    fps = periods_total / average_count;

    tumFontSelectFontFromName(FPS_FONT);

    sprintf(str, "FPS: %2d", fps);

    if (!tumGetTextSize((char *)str, &text_width, NULL))
        tumDrawText(str, SCREEN_WIDTH - text_width - 40,
                              SCREEN_HEIGHT - DEFAULT_FONT_SIZE - 40,
                              Skyblue);

    tumFontSelectFontFromHandle(cur_font);
    tumFontPutFontHandle(cur_font);
}

void appendToStr(char *theString, char theCharacter)
{
    int iterator;
    //Go through the string until you reach the end
    for(iterator = 0; theString[iterator] != 0; iterator++);
    //Append the character
    theString[iterator] = theCharacter;
}