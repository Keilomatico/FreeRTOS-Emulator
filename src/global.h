/**
 * @file global.h
 * @author Adrian Keil
 * @date 02 December 2020
 * @brief This is the global header, which contains 
 * all globally shared variables and structures
 */

#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>

#include <SDL2/SDL_scancode.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "TUM_Ball.h"
#include "TUM_Draw.h"
#include "TUM_Event.h"
#include "TUM_Sound.h"
#include "TUM_Utils.h"
#include "TUM_Font.h"

#include "AsyncIO.h"

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