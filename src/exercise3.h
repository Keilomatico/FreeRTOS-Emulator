/**
 * @file exercise3.h
 * @author Adrian Keil
 * @brief Contains all the functions for exercise 3
 */

#ifndef __Excercise3_H__
#define __Excercise3_H__

#include "global.h"
#include "miscFunc.h"

#define TASK3A_INTERVAL     500
#define TASK3B_INTERVAL     250
#define COUNTER_INTERVAL    1000
#define COUNTER_RESET       15000
#define CIRCLE_RADIUS       30
#define CENTER_OFFSET       80
#define BUTTON_TEXT_X       30
#define BUTTON_TEXT1_Y      20
#define BUTTON_TEXT2_Y      40
#define BUTTON_TEXT3_Y      60

#define BIT_CIRCLE1_ON      0x01
#define BIT_CIRCLE1_OFF     0x02
#define BIT_CIRCLE2_ON      0x04
#define BIT_CIRCLE2_OFF     0x08
#define BIT_BUTTON2         0x02
#define BIT_RESET_COUNTER   0x01
#define BIT_UPDATE_TIME     0x01

/**
 * @brief Enter function for exercise 3.
 */
void exercise3enter(void *data);

/**
 * @brief Exit function for exercise 3.
 */
void exercise3exit(void *data);

/**
 * @brief Handles all the actual drawing to the screen.
 */
void vExercise3draw(void *pvParameters);

/**
 * @brief Notifies the drawing task when to draw the left circle.
 */
void vExercise3circle1(void *pvParameters);

/**
 * @brief Notifies the drawing task when to draw the right circle.
 */
void vExercise3circle2(void *pvParameters);

/**
 * @brief Handles counting button 1. 
 */
void vExercise3button1(void *pvParameters);

/**
 * @brief Handles counting button 2.
 */
void vExercise3button2(void *pvParameters);

/**
 * @brief Increments a counter. 
 * Can be reset by setting its notification value appropriately.
 */
void vExercise3count(void *pvParameters);

/**
 * @brief Callback for the Exercise3timer. 
 * Notifies Exercise3button1 and 2 to reset their counters.
 */
void vExercise3timerCallback(TimerHandle_t xTimer);

#endif