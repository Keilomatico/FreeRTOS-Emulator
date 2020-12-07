/**
 * @file exercise4.h
 * @author Adrian Keil
 * @brief Contains all the functions for exercise 4
 */

#ifndef __Excercise4_H__
#define __Excercise4_H__

#include "global.h"
#include "miscFunc.h"

//Length 30 should be fine for a short string like ("Tick 12: Task 3 running"
#define EX4_QUEUE_SIZE      30
#define POS_X               30
#define POS_Y               30
#define TICK_NUM            15
#define EX4_ARRAY_LENGTH    50

#define BIT_NEW_CALL        0x01
#define BIT_TASK1_RUNNING   0x02
#define BIT_TASK2_RUNNING   0x04
#define BIT_TASK3_RUNNING   0x08
#define BIT_TASK4_RUNNING   0x10
#define SHIFT_TASK1_RUNNING 1
#define SHIFT_TASK2_RUNNING 2
#define SHIFT_TASK3_RUNNING 3
#define SHIFT_TASK4_RUNNING 4

void exercise4enter(void *data);

void exercise4exit(void *data);

void vExercise4draw(void *pvParameters);

void vExercise4task1(void *pvParameters);

void vExercise4task2(void *pvParameters);

void vExercise4task3(void *pvParameters);

void vExercise4task4(void *pvParameters);

#endif