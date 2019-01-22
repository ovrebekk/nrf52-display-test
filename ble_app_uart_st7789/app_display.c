#include "app_display.h"
#include "ugui.h"
#include "nrf_gfx_ext.h"
#include "images.h"
#include <string.h>

UG_WINDOW window_1;

extern const nrf_lcd_t nrf_lcd_st7789;
static const nrf_lcd_t * p_lcd = &nrf_lcd_st7789;

const UG_COLOR display_app_state_button_color[] = APP_STATE_COLORS;
const UG_COLOR display_app_state_button_font_color[] = APP_STATE_FONT_COLORS;
const UG_COLOR display_on_off_color[] = ON_OFF_COLORS;
const UG_COLOR display_on_off_font_color[] = ON_OFF_FONT_COLORS;

UG_GUI gui;
UG_TEXTBOX textbox_val_1;
UG_TEXTBOX textbox_val_2;
UG_BUTTON button_val_1;
UG_BUTTON button_val_2;
UG_IMAGE  image_1;

#define MAX_OBJECTS 20

UG_OBJECT obj_buff_wnd_1[MAX_OBJECTS];

static app_display_content_t content_previous = {0};

void window_1_callback (UG_MESSAGE *msg)
{
    UNUSED_PARAMETER(msg);    
}

void app_display_init(app_display_content_t *initial_state)
{
    content_previous = *initial_state;
    UG_Init(&gui, 240, 240, p_lcd);
}

void app_display_create_main_screen(app_display_content_t *content)
{
    /* Create the window */
    UG_WindowCreate (&window_1, obj_buff_wnd_1, MAX_OBJECTS, window_1_callback) ;
    
    /* Modify the window t i t l e */
    UG_WindowSetTitleText(&window_1, content->main_title) ;
    UG_WindowSetTitleTextFont(&window_1, &FONT_10X16) ;
    UG_WindowSetTitleTextAlignment(&window_1, ALIGN_CENTER);

    /* Create "Toggle PHY" textbox (TXB_ID_0) */
    UG_TextboxCreate(&textbox_val_1, &window_1, TXT_ID_0_X_LOCATION, TXT_ID_0_Y_LOCATION, TXT_ID_0_X_LOCATION+TXT_ID_0_WIDTH, TXT_ID_0_Y_LOCATION+TXT_ID_0_HEIGHT);  
    UG_TextboxSetFont(&textbox_val_1, &FONT_8X12);
    UG_TextboxSetText(&textbox_val_1 , "Temperature") ;
    UG_TextboxSetForeColor(&textbox_val_1, FONT_COLOR_TEXT) ;
    UG_TextboxSetBackColor(&textbox_val_1, FILL_COLOR_TEXT); 
    UG_TextboxSetAlignment(&textbox_val_1, ALIGN_CENTER);

    /* Create "PHY" selection button (BTN_ID_0) */
    UG_ButtonCreate(&button_val_1, &window_1, BTN_ID_0_X_LOCATION, BTN_ID_0_Y_LOCATION, BTN_ID_0_X_LOCATION+BTN_ID_0_WIDTH, BTN_ID_0_Y_LOCATION+BTN_ID_0_HEIGHT);
    UG_ButtonSetStyle(&button_val_1, BTN_STYLE_3D|BTN_STYLE_USE_ALTERNATE_COLORS);
    UG_ButtonSetForeColor(&button_val_1, FONT_COLOR_BUTTON);
    UG_ButtonSetBackColor(&button_val_1, FILL_COLOR_BUTTON);  
    UG_ButtonSetFont(&button_val_1, &FONT_10X16);

    /* Create "Toggle output power" textbox (TXB_ID_1) */
    UG_TextboxCreate(&textbox_val_2, &window_1, TXT_ID_1_X_LOCATION, TXT_ID_1_Y_LOCATION, TXT_ID_1_X_LOCATION+TXT_ID_1_WIDTH, TXT_ID_1_Y_LOCATION+TXT_ID_1_HEIGHT);  
    UG_TextboxSetFont(&textbox_val_2, &FONT_8X12);
    UG_TextboxSetText(&textbox_val_2, "Voltage") ;
    UG_TextboxSetForeColor (&textbox_val_2, FONT_COLOR_TEXT) ;
    UG_TextboxSetBackColor (&textbox_val_2, FILL_COLOR_TEXT); 
    UG_TextboxSetAlignment (&textbox_val_2, ALIGN_CENTER);  

    /* Create "Power" selection button (BTN_ID_1) */
    UG_ButtonCreate(&button_val_2, &window_1, BTN_ID_1_X_LOCATION, BTN_ID_1_Y_LOCATION, BTN_ID_1_X_LOCATION+BTN_ID_1_WIDTH, BTN_ID_1_Y_LOCATION+BTN_ID_1_HEIGHT);
    UG_ButtonSetStyle(&button_val_2, BTN_STYLE_3D|BTN_STYLE_USE_ALTERNATE_COLORS);
    UG_ButtonSetForeColor(&button_val_2, FONT_COLOR_BUTTON);
    UG_ButtonSetBackColor(&button_val_2, FILL_COLOR_BUTTON);  
    UG_ButtonSetFont(&button_val_2, &FONT_10X16);

    UG_ImageCreate(&image_1, &window_1, 0, 128, 30, 150);
    UG_ImageSetBMP(&image_1, &bmp_nordicsemi);

    /* Update the dynamic elements of the screen */
    app_display_update_main_screen(content);

    /* Finally , show the window */
    UG_WindowShow(&window_1) ;
}

static char sprintf_buf[64];

void app_display_update_main_screen(app_display_content_t *content)
{
    static bool first_update = true;
    static app_display_content_t prev_content;
    if(first_update || prev_content.val_1 != content->val_1)
    {
        static char val_1_string[16];
        sprintf(val_1_string, "%i.%i C", (content->val_1 / 10), (content->val_1 % 10));
        UG_ButtonSetText(&button_val_1, val_1_string);
    }
    if(first_update || prev_content.val_2 != content->val_2)
    {
        static char val_2_string[16];
        sprintf(val_2_string, "%i.%i V", (content->val_2 / 10), (content->val_2 % 10));
        UG_ButtonSetText(&button_val_2, val_2_string);
    }
}

void app_display_update(void)
{
    UG_Update();
}
