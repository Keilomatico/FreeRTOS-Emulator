#include "exercise2.h"

void exercise2run(void *data)
{
    printf("Resuming task 2 \n");
    vTaskResume(Exercise2);
}

void exercise2exit(void *data)
{
    printf("Suspending task 2 \n");
    vTaskSuspend(Exercise2);
}


void vExercise2(void *pvParameters)
{
    static char my_string[100]; // structure to store my text
    static int my_string_width = 0;
    static coord_t mycoordinates[4];
    static float i=0;
    static int offset_x;
    static int offset_y;
    static int counter[4] = { 0 };
    TickType_t last_change[4] = { 0 };
    

    while (1) {
        if (DrawSignal) {
            if (xSemaphoreTake(DrawSignal, portMAX_DELAY) == pdTRUE) {
                xGetButtonInput(); // Update global input

                // `buttons` is a global shared variable and as such needs to be
                // guarded with a mutex, mutex must be obtained before accessing the
                // resource and given back when you're finished. If the mutex is not
                // given back then no other task can access the reseource.
                if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
                    if (buttons.buttons[KEYCODE(
                                            Q)]) { // Equiv to SDL_SCANCODE_Q
                        exit(EXIT_SUCCESS);
                    }
                    xSemaphoreGive(buttons.lock);
                }

                if(tumEventGetMouseMiddle())
                {
                    counter[INDEX_A] = 0;
                    counter[INDEX_B] = 0;
                    counter[INDEX_C] = 0;
                    counter[INDEX_D] = 0;
                }

                counter[INDEX_A] += checkbutton(&last_change[INDEX_A], KEYCODE(A));
                counter[INDEX_B] += checkbutton(&last_change[INDEX_B], KEYCODE(B));
                counter[INDEX_C] += checkbutton(&last_change[INDEX_C], KEYCODE(C));
                counter[INDEX_D] += checkbutton(&last_change[INDEX_D], KEYCODE(D));

                xSemaphoreTake(ScreenLock, portMAX_DELAY);
                
                tumDrawClear(White); // Clear screen

                //Move the entire screen with the mouse
                tumDrawSetGlobalXOffset(tumEventGetMouseX()/MOUSE_OFFSET_DEVIDER);
                tumDrawSetGlobalYOffset(tumEventGetMouseY()/MOUSE_OFFSET_DEVIDER);

                //Display the mouse coordinates
                sprintf(my_string, "A: %d | B: %d | C: %d | D: %d", 
                    counter[INDEX_A], counter[INDEX_B], counter[INDEX_C], counter[INDEX_D]);
                tumDrawText(my_string,
                                BUTTON_TEXT_POS_X,
                                BUTTON_TEXT_POS_Y,
                                Black);

                sprintf(my_string, "Mouse position: X: %d | Y: %d", 
                    tumEventGetMouseX(), tumEventGetMouseX()); 
                    tumDrawText(my_string,
                                20,
                                10,
                                Black);

                //Calculate offset for rotating parts
                offset_x = (int) (ROTATION_RADIUS * cos(i));
                offset_y = (int) (ROTATION_RADIUS * sin(i));
        
                //Draw the Circle
                //tumDrawArc is unsed instead of tumDrawCircle to create 
                //just the outline and not a filled circle
                tumDrawArc(  SCREEN_WIDTH / 2 - offset_x, 
                                SCREEN_HEIGHT / 2 + offset_y,
                                MYCIRCLE_RADIUS,
                                0,
                                359,
                                Purple);

                //Create the points for the triangle
                getTriangleCoordinates(mycoordinates, SCREEN_WIDTH / 2, 
                SCREEN_HEIGHT / 2, TRIANGLE_WIDTH, TRIANGLE_HEIGHT);

                tumDrawPoly(mycoordinates, 3, Teal);  //Draw the triangle


                //Create the points for the square
                getSquareCorrdinates(mycoordinates, SCREEN_WIDTH / 2 + offset_x, 
                SCREEN_HEIGHT / 2 - offset_y, SQUARE_LENGTH);

                tumDrawPoly(mycoordinates, 4, Fuchsia);  //Draw the sqare

                // Format the string into the char array
                sprintf(my_string, "Hello ESPL");
                // Get the width of the string on the screen so we can center it
                // Returns 0 if width was successfully obtained
                if (!tumGetTextSize((char *)my_string, &my_string_width, NULL))
                    tumDrawText(my_string,
                                SCREEN_WIDTH / 2 - my_string_width / 2,
                                SCREEN_HEIGHT / 2 - DEFAULT_FONT_SIZE / 2 + TEXT_OFFSET_Y,
                                TUMBlue);
                
                
                //Same as above
                sprintf(my_string, "This is exercise 2.1");
                if (!tumGetTextSize((char *)my_string, &my_string_width, NULL))
                {
                    //Calculate the offset for the text (offset_x is reused here)
                    offset_x = (int) (offset_x * (SCREEN_WIDTH / 2 - my_string_width / 2)
                     / ROTATION_RADIUS); 
                    tumDrawText(my_string,
                                SCREEN_WIDTH / 2 - my_string_width / 2 + offset_x,
                                SCREEN_HEIGHT / 2 - DEFAULT_FONT_SIZE / 2 - TEXT_OFFSET_Y,
                                TUMBlue);
                }

                xSemaphoreGive(ScreenLock);

                if(i >= 2*M_PI)
                    i=0;
                else
                    i += 0.02;
            }
        }
    }
}