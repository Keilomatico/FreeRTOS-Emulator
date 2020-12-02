#include "TUM_Ball.h"
#include "TUM_Draw.h"
#include "TUM_Event.h"
#include "TUM_Sound.h"
#include "TUM_Utils.h"
#include "TUM_Font.h"

#include "miscFunc.h"
#include "global.h"

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

void vExercise2(void *pvParameters);