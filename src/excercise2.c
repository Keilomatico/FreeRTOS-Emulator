#include "exercise2.h"


void exercise2enter(void *data)
{
    printf("Resuming task 2 \n");
    xSemaphoreGive(exercise2Mutex);
    vTaskResume(Exercise2);
}

void exercise2exit(void *data)
{
    printf("Suspending task 2 \n");
    //Take the semaphore and block indefinitely to make sure that the task
    //has reached its end and is not holdig any semaphores or mutexes
    xSemaphoreTake(exercise2Mutex, portMAX_DELAY);
    vTaskSuspend(Exercise2);
}

void vExercise2(void *pvParameters)
{
    char my_string[100];        //array to temporarily store text
    int my_string_width = 0;    //temporarily store the width of the text
    coord_t mycoordinates[4];   //used to temporarily store some coordinates
    static float rad=0;         //is incremented from 0 to 2*pi to calc the offset
    int offset_x;               //used to temporarily store the offset in x direction
    int offset_y;               //used to temporarily store the offset in y direction
    static int counter[4] = { 0 };  //Counts the number of times a button has been pressed
    TickType_t last_change[4];  //Stores the timestamp of the last change of a button

    for(int i=0; i<4; i++) {
        last_change[i] = xTaskGetTickCount();
    }

    while (1) {
        if (DrawSignal) {   //Check if the Semaphore has been initialized
            xSemaphoreTake(exercise2Mutex, portMAX_DELAY);
            xSemaphoreTake(DrawSignal, portMAX_DELAY);
            xGetButtonInput(); // Update global input

            //Middle Mouse button is used to reset all counter values
            if(tumEventGetMouseMiddle())
            {
                counter[INDEX_A] = 0;
                counter[INDEX_B] = 0;
                counter[INDEX_C] = 0;
                counter[INDEX_D] = 0;
            }

            //Update the counter values
            counter[INDEX_A] += checkbutton(&last_change[INDEX_A], KEYCODE(A));
            counter[INDEX_B] += checkbutton(&last_change[INDEX_B], KEYCODE(B));
            counter[INDEX_C] += checkbutton(&last_change[INDEX_C], KEYCODE(C));
            counter[INDEX_D] += checkbutton(&last_change[INDEX_D], KEYCODE(D));

            //Take the ScreenLock to start drawing
            xSemaphoreTake(ScreenLock, portMAX_DELAY);
            
            tumDrawClear(White); // Clear screen
            vDrawFPS();
            
            //Move the entire screen with the mouse
            tumDrawSetGlobalXOffset(tumEventGetMouseX()/MOUSE_OFFSET_DEVIDER);
            tumDrawSetGlobalYOffset(tumEventGetMouseY()/MOUSE_OFFSET_DEVIDER);

            //Print the button presses
            sprintf(my_string, "A: %d | B: %d | C: %d | D: %d", 
                counter[INDEX_A], counter[INDEX_B], counter[INDEX_C], counter[INDEX_D]);
            tumDrawText(my_string,
                            BUTTON_TEXT_POS_X,
                            BUTTON_TEXT_POS_Y,
                            Black);
            //Print the mouse coordinates
            sprintf(my_string, "Mouse position: X: %d | Y: %d", 
                tumEventGetMouseX(), tumEventGetMouseX()); 
                tumDrawText(my_string,
                            20,
                            10,
                            Black);

            //Calculate offset for rotating parts
            offset_x = (int) (ROTATION_RADIUS * cos(rad));
            offset_y = (int) (ROTATION_RADIUS * sin(rad));
    
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
            //Draw the triangle
            tumDrawPoly(mycoordinates, 3, Teal);  

            //Create the points for the square
            getSquareCorrdinates(mycoordinates, SCREEN_WIDTH / 2 + offset_x, 
            SCREEN_HEIGHT / 2 - offset_y, SQUARE_LENGTH);
            //Draw the sqare
            tumDrawPoly(mycoordinates, 4, Fuchsia);

            // Print Hello ESPL and center it on the X axis
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

            //Drawing is done -> give the ScreenLock back
            xSemaphoreGive(ScreenLock);

            //Rad is incremented from 0 until it reaches 2*pi
            if(rad >= 2*M_PI)
                rad=0;
            else
                rad += 0.03;

            xSemaphoreGive(exercise2Mutex);
        }
    }
}