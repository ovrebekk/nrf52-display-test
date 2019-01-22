#ifndef __APP_DISPLAY_H
#define __APP_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>

#define SCREEN_Y_LOC_MID            115

#define INTERWIDGET_SPACE           4
#define GROUPED_INTERWIDGET_SPACE   2

#define RGB_888_TO_565(a) ((((a) & 0xF80000) >> 8) | (((a) & 0x00FC00) >> 5) | (((a) & 0x0000FC) >> 3))

// Phy text/button
#define TXT_ID_0_X_LOCATION         4
#define TXT_ID_0_Y_LOCATION         5
#define TXT_ID_0_WIDTH              105
#define TXT_ID_0_HEIGHT             20

#define BTN_ID_0_X_LOCATION TXT_ID_0_X_LOCATION
#define BTN_ID_0_Y_LOCATION TXT_ID_0_Y_LOCATION + TXT_ID_0_HEIGHT + GROUPED_INTERWIDGET_SPACE
#define BTN_ID_0_WIDTH TXT_ID_0_WIDTH
#define BTN_ID_0_HEIGHT 30

// TX power text/button
#define TXT_ID_1_X_LOCATION TXT_ID_0_X_LOCATION + SCREEN_Y_LOC_MID
#define TXT_ID_1_Y_LOCATION TXT_ID_0_Y_LOCATION
#define TXT_ID_1_WIDTH TXT_ID_0_WIDTH
#define TXT_ID_1_HEIGHT TXT_ID_0_HEIGHT

#define BTN_ID_1_X_LOCATION TXT_ID_1_X_LOCATION
#define BTN_ID_1_Y_LOCATION TXT_ID_0_Y_LOCATION + TXT_ID_1_HEIGHT + GROUPED_INTERWIDGET_SPACE
#define BTN_ID_1_WIDTH TXT_ID_0_WIDTH
#define BTN_ID_1_HEIGHT BTN_ID_0_HEIGHT

#define FONT_COLOR_TEXT         C_WHITE
#define FILL_COLOR_TEXT         C_DODGER_BLUE
#define FONT_COLOR_BUTTON       C_YELLOW
#define FILL_COLOR_BUTTON       C_MEDIUM_BLUE
#define APP_STATE_COLORS        {C_SILVER, C_MEDIUM_SPRING_GREEN, C_DARK_GREEN, C_RED}
#define APP_STATE_FONT_COLORS   {C_BLACK, C_BLACK, C_YELLOW, C_YELLOW}
#define ON_OFF_COLORS           {C_SILVER, C_MEDIUM_BLUE}
#define ON_OFF_FONT_COLORS      {C_BLACK, C_YELLOW}

typedef enum {APP_STATE_IDLE, APP_STATE_ADVERTISING, APP_STATE_CONNECTED, APP_STATE_DISCONNECTED} app_state_t;
typedef enum {APP_PHY_CODED, APP_PHY_1M, APP_PHY_2M, APP_PHY_MULTI, APP_PHY_LIST_END} app_phy_t;
    
typedef struct{
    char *main_title;
    uint32_t val_1;
    uint32_t val_2;
}app_display_content_t;

void app_display_init(app_display_content_t *initial_state);

void app_display_create_main_screen(app_display_content_t *content);

void app_display_update_main_screen(app_display_content_t *content);

void app_display_update(void);

#endif
