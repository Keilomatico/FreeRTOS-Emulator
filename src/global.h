#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#define KEYCODE(CHAR) SDL_SCANCODE_##CHAR

#define DEBOUNCE_DELAY 300

#ifndef GLOBAL_H
#define GLOBAL_H
struct buttons_buffer {
    unsigned char buttons[SDL_NUM_SCANCODES];
    SemaphoreHandle_t lock;
};
#endif

extern SemaphoreHandle_t DrawSignal;
extern SemaphoreHandle_t ScreenLock;
extern struct buttons_buffer buttons;