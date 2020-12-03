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
#define CIRCLE_RADIUS       30
#define CENTER_OFFSET       80

void exercise3enter(void *data);

void exercise3exit(void *data);

void vExercise3a(void *pvParameters);

void vExercise3b(void *pvParameters);

#endif