
#ifndef _LVGL_DISPLAY_
#define _LVGL_DISPLAY_

#include <lvgl.h>

//TFT LVGL Objects
static lv_obj_t * scr;                // Main Screen object

static lv_obj_t * scr_home;
static lv_obj_t * scr_settings;
static lv_obj_t * scr_view_messages;
static lv_obj_t * scr_write_message;
static lv_obj_t *label_time_date;


// Settings Screen
static lv_obj_t *label_settings_WIFI_status;
static lv_obj_t *ta_ssid;
static lv_obj_t *ta_password;
static lv_obj_t * wifi_scan_list;



// Functions
void LvglInit();
extern void UpdateWifiStatusLabelText(String text);
extern void UpdateWifiScanDropDown(String text);
extern void UpdateTimeDateLabelText(String text);


#endif






