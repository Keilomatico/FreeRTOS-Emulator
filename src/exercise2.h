/**
 * @file exercise2.h
 * @author Adrian Keil
 * @brief Contains all the functions for exercise 2
 */

#ifndef __Excercise2_H__
#define __Excercise2_H__

#include "global.h"
#include "miscFunc.h"

#define INDEX_A    0
#define INDEX_B    1
#define INDEX_C    2
#define INDEX_D    3

#define ROTATION_RADIUS         100
#define MYCIRCLE_RADIUS         30
#define TRIANGLE_WIDTH          70
#define TRIANGLE_HEIGHT         60     
#define SQUARE_LENGTH           60
#define TEXT_OFFSET_Y           150
#define BUTTON_TEXT_POS_X       20
#define BUTTON_TEXT_POS_Y       30
#define MOUSE_OFFSET_DEVIDER    10

/**
 * @brief Enter function for exercise 2. 
 */
void exercise2enter(void *data);

/**
 * @brief Exit function for exercise 2. 
 */
void exercise2exit(void *data);

/**
 * @brief Task function for exercise 2.
 *        Handles calculation, drawing and button checking
 * @param pvParameters Not used but necessary for task creation
 */
void vExercise2(void *pvParameters);

#endif