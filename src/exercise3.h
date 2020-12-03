/**
 * @file exercise3.h
 * @author Adrian Keil
 * @brief Contains all the functions for exercise 3
 */

#ifndef __Excercise3_H__
#define __Excercise3_H__

#include "global.h"
#include "miscFunc.h"

#define TASK3A_INTERVAL     1000
#define TASK3B_INTERVAL     500
#define COUNTER_INTERVAL    1000
#define CIRCLE_RADIUS       30
#define CENTER_OFFSET       80
#define BUTTON_TEXT_X       30
#define BUTTON_TEXT1_Y      20
#define BUTTON_TEXT2_Y      40
#define BUTTON_TEXT3_Y      60

#define BUTTON2_BIT         0x01
#define BIT_CIRCLE1_ON      0x01
#define BIT_CIRCLE1_OFF     0x02
#define BIT_CIRCLE2_ON      0x04
#define BIT_CIRCLE2_OFF     0x08
#define BIT_UPDATE_TIME     0x01

void exercise3enter(void *data);

void exercise3exit(void *data);

void vExercise3draw(void *pvParameters);

void vExercise3circle1(void *pvParameters);

void vExercise3circle2(void *pvParameters);

void vExercise3button1(void *pvParameters);

void vExercise3button2(void *pvParameters);

void vExercise3count(void *pvParameters);

#endif