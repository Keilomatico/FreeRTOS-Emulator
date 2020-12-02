/**
 * @file global.h
 * @author Adrian Keil
 * @date 02 December 2020
 * @brief This is the global header, which contains 
 * all globally shared variables and structures
 */

#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#define KEYCODE(CHAR) SDL_SCANCODE_##CHAR

#define DEBOUNCE_DELAY 300

typedef struct buttons_buffer {
    unsigned char buttons[SDL_NUM_SCANCODES];
    SemaphoreHandle_t lock;
}buttons_buffer_t;

extern SemaphoreHandle_t DrawSignal;
extern SemaphoreHandle_t ScreenLock;
extern buttons_buffer_t buttons;


#endif