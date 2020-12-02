#include "miscFunc.h"

void xGetButtonInput(void)
{
    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
        xQueueReceive(buttonInputQueue, &buttons.buttons, 0);
        xSemaphoreGive(buttons.lock);
    }
}

int checkbutton(TickType_t *last_change, int keycode)
{
    TickType_t current_tick;
    static int ret;

    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
        current_tick = xTaskGetTickCount();
        if (buttons.buttons[keycode] > 0 && current_tick - *last_change > DEBOUNCE_DELAY) {
            *last_change = current_tick;
            buttons.buttons[keycode] = 0;
            ret = 1;
        }
        else
            ret = 0;
        xSemaphoreGive(buttons.lock);
    }
    else
        ret = 0;
    return ret;   
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