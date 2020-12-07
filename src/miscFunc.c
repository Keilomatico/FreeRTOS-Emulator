#include "miscFunc.h"

void xGetButtonInput(void)
{
    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
        xQueueReceive(buttonInputQueue, &buttons.buttons, 0);
        xSemaphoreGive(buttons.lock);
    }
}

int checkbutton(TickType_t *last_pressed, int keycode)
{
    TickType_t current_tick;
    int ret = 0;

    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
        current_tick = xTaskGetTickCount();
        //Debounce: Checks if the time in between two button presses has been long enough
        if (buttons.buttons[keycode] > 0) {
	        if(current_tick - *last_pressed > DEBOUNCE_DELAY)
	            ret = 1;
            *last_pressed = current_tick;
            //Reset the button value in the struct so it can be pressed again
            buttons.buttons[keycode] = 0;
        }
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