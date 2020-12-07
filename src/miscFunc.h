/**
 * @file miscFunc.h
 * @author Adrian Keil
 * @brief Contains a variety of functions, which might me useful in multiple files
 */

#ifndef __MISC_H__
#define __MISC_H__

#include "global.h"

#define FPS_AVERAGE_COUNT 50
#define FPS_FONT "IBMPlexSans-Bold.ttf"

/**
 * @brief Updates the buttonInputQueue
 */
void xGetButtonInput(void);

/**
 * @brief Checks if a specific button has been pressed and handles debouncing
 *
 * @param Keycode SDL_Scancode of the Key
 * @return 1 if the button has been pressed, 0 if not or if the Semaphore couldn't have been taken
 */
int checkbutton(int keycode);

/**
 * @brief Calculates the coordinates for a rectangle
 *
 * @param coordinates Points array to return the calculated points (must be at least of length 4)
 * @param x X coordinate of the center of the square
 * @param y Y coordinate of the center of the square
 * @param x_length Length of x side of the square in pixels
 * @param y_length Length of y side of the square in pixels
 * @return 0 on success
 */
int getRectCorrdinates(coord_t *coordinates, int x, int y, int x_length, int y_length);

/**
 * @brief Calculates the coordinates for a square
 *
 * @param coordinates Points array to return the calculated points (must be at least of length 4)
 * @param x X coordinate of the center of the square
 * @param y Y coordinate of the center of the square
 * @param length Length of each side of the square in pixels
 * @return 0 on success
 */
int getSquareCorrdinates(coord_t *coordinates, int x, int y, int length);

/**
 * @brief Calculates the coordinates for a triangle
 *
 * @param coordinates Points array to return the calculated points (must be at least of length 3)
 * @param x X coordinate of the center of the square
 * @param y Y coordinate of the center of the square
 * @param width Length of the base side in pixels
 * @param height Height of the triangle in pixels
 * @return 0 on success
 */
int getTriangleCoordinates(coord_t *coordinates, int x, int y, int width, int height);

/**
 * @brief Draws the current framerate on the screen
 */
void vDrawFPS(void);

#endif