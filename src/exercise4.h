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
#define POS_X               30
#define POS_Y               30
#define TICK_NUM            15
#define EX4_ARRAY_LENGTH    50

#define BIT_NEW_CALL        0x01
#define SHIFT_TASK1_RUNNING 1
#define SHIFT_TASK2_RUNNING 2
#define SHIFT_TASK3_RUNNING 3
#define SHIFT_TASK4_RUNNING 4
#define BIT_TASK1_RUNNING   (0x01 << SHIFT_TASK1_RUNNING)
#define BIT_TASK2_RUNNING   (0x01 << SHIFT_TASK2_RUNNING)
#define BIT_TASK3_RUNNING   (0x01 << SHIFT_TASK3_RUNNING)
#define BIT_TASK4_RUNNING   (0x01 << SHIFT_TASK4_RUNNING)

#define EX4_DRAW_PRIO       5
#define EX4_TASK1_PRIO      1
#define EX4_TASK2_PRIO      2
#define EX4_TASK3_PRIO      3
#define EX4_TASK4_PRIO      4

void exercise4enter(void *data);

void exercise4exit(void *data);

void vExercise4draw(void *pvParameters);

void vExercise4task1(void *pvParameters);

void vExercise4task2(void *pvParameters);

void vExercise4task3(void *pvParameters);

void vExercise4task4(void *pvParameters);

#endif