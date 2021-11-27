/*	Project : Read Google Spread Sheet Data from ESP32	*/
/*Refer following video for complete project : https://youtu.be/0LoeaewIAdY*/


#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Ping.h>

#include <Wire.h>
#include <SPI.h>

// TFT Display Libraries 
#include <lvgl.h>
#include <TFT_eSPI.h>

// Definitions
#define MODE_READ_CELL_RANGE 0
#define MODE_READ_LAST_CELL 1

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240



// Interrupt Timer Definitions
#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     0
#include "ESP32TimerInterrupt.h"
#define TIMER0_INTERVAL_MS        20000
void IRAM_ATTR TimerHandler0(void);

/********************************************************************************/
//Things to change
const char * ssid = "dlink-80EB";
const char * password = "abc123abc";

String GOOGLE_SCRIPT_ID = "AKfycbwX8S-OX1MyQfS8jirMaF7FK2M1sBkYoVzDVRl3MpQibCIaqOGeq1TdCT8J6qhEY_oh";

const int sendInterval = 5000; 
/********************************************************************************/

// Google Sheets Data Variables 
String dataDateString, dataTimeString, dataMessageString;
int dataSentFlag, dataReceivedFlag = 0;
int checkGoogleSheetsDataFlag = 1;

// TFT LVGL Variables
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

//TFT LVGL Objects
static lv_obj_t *label;
static lv_obj_t * kb;
static lv_obj_t * tb_message;
static lv_obj_t * bg_left_panel;
static lv_obj_t * bg_right_panel;
static lv_obj_t * btn_write_message;
static lv_obj_t * ta_write_message;


TaskHandle_t ntScanTaskHandler;
TaskHandle_t ntConnectTaskHandler;

// Timer Interrupt Variables
volatile uint32_t Timer0Count = 0;
ESP32Timer ITimer0(0);

// Initialise Instances
WiFiClientSecure client;
TFT_eSPI tft = TFT_eSPI();    /* TFT instance */

// Setup
void setup() 
{
  // Initialise serial port
  Serial.begin(115200);                       
  delay(10);

  // Connect to the wifi
  WiFi.mode(WIFI_STA);                        
  WiFi.begin(ssid, password);
  Serial.println("Started");
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Ready to go");

  // Initialise TFT Display
  //InitTFTDisplay();

    xTaskCreate(
    guiTask,    // Function that should be called
    "gui",   // Name of the task (for debugging)
    4096*2,            // Stack size (bytes)
    NULL,            // Parameter to pass
    2,               // Task priority
    NULL             // Task handle
  );

  vTaskDelay(500);

    xTaskCreate(
    scanGSheetTask,    // Function that should be called
    "ScanGSheetTask",   // Name of the task (for debugging)
    4096*3,            // Stack size (bytes)
    NULL,            // Parameter to pass
    1,               // Task priority
    &ntScanTaskHandler             // Task handle
  );


  // Initialise Timer0 interrupt
  //ITimer0.attachInterruptInterval(TIMER0_INTERVAL_MS * 1000, TimerHandler0);


}

// Interrupt every TIMER0_INTERVAL_MS to check for updated data from google sheets
// void TimerHandler0()
// {
//   checkGoogleSheetsDataFlag = 1;
// }

// Main loop
void loop() 
{
  // if(checkGoogleSheetsDataFlag == 1)
  // {
  //   ReadDataFromSheets();
  //   checkGoogleSheetsDataFlag = 0;

  //   lv_label_set_text(label, dataMessageString.c_str());
  //   lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
    
  // }
  
  //   lv_task_handler(); /* let the GUI do its work */
  //   delay(5);
}



// Google Sheets Data Functions
/********************************************************************************/

void scanGSheetTask(void *pvParameters) 
{ 

  while(1)
  {
    vTaskDelay(10000);
    ReadDataFromSheets();  
    lv_label_set_text(label, dataMessageString.c_str());
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
  }

    
}


void ReadDataFromSheets()
{
  char parsedData[100];
  int commaIndex[10] = {0};
  String spreadSheetReturnString;
  int counter = 0;
  spreadSheetReturnString = readSpreadSheet(MODE_READ_LAST_CELL);
  for(counter = 1; counter <= 5; counter++)
  {
    commaIndex[counter] = spreadSheetReturnString.indexOf(',', commaIndex[counter - 1] + 1);
  }


  dataDateString = spreadSheetReturnString.substring(0, commaIndex[1]);
  dataTimeString = spreadSheetReturnString.substring(commaIndex[1] + 1, commaIndex[2]);
  dataMessageString = spreadSheetReturnString.substring(commaIndex[2] + 1, commaIndex[3]);
  dataSentFlag = (spreadSheetReturnString.substring(commaIndex[3] + 1, commaIndex[4])).toInt();
  dataReceivedFlag = (spreadSheetReturnString.substring(commaIndex[4] + 1, commaIndex[5])).toInt();
  
  
  /* Debug received message */
  sprintf(parsedData,"Date: %s, Time: %s, Message: %s, Sent Flag: %i, Received Flag: %i", dataDateString, dataTimeString, dataMessageString, dataSentFlag, dataReceivedFlag);
  // Serial.println(dataDateString);
  // Serial.println(dataTimeString);
  // Serial.println(dataMessageString);
  // Serial.println(dataSentFlag);
  // Serial.println(dataReceivedFlag);
   
  Serial.println(parsedData);
}

String readSpreadSheet(byte modeType)
{
  String url = "";
  //https://script.google.com/macros/s/AKfycbwX8S-OX1MyQfS8jirMaF7FK2M1sBkYoVzDVRl3MpQibCIaqOGeq1TdCT8J6qhEY_oh/exec?mode=0&rangestart=C&rangestart=3&rangeend=C&rangeend=6
   HTTPClient http;

  switch(modeType)
  {
    case(MODE_READ_LAST_CELL):
      url="https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?mode=" + modeType;
    break;


    
  }
   
  Serial.println(url);
  Serial.println("Making a request");
  http.begin(url.c_str()); //Specify the URL and certificate
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();
  String payload;
    if (httpCode > 0) { //Check for the returning code
        payload = http.getString();
        
        //Serial.println(httpCode);
        //Serial.println(payload);
      }
    else {
      Serial.println("Error on HTTP request");
    }
  http.end();

  return payload;
}

void writeSpreadSheet()
{
     HTTPClient http;
   String url="https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec";
//   Serial.print(url);
  Serial.print("Making a request");
  http.begin(url.c_str()); //Specify the URL and certificate
  //http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.addHeader("Content-Type", "text/plain");
  //int httpCode = http.GET();
  char dataTest[50];
  int cellRow = 3;
  String cellColumn = "A";
  String cellValue = "Hello123";
  sprintf(dataTest,"cellrow=%i&cellcolumn=A&cellvalue=%s",cellRow,cellColumn,cellValue);
  Serial.println(dataTest);
  int httpCode = http.POST("test=99");
  //int httpCode = http.POST("cellrow=3&cellcolumn=A&cellvalue=A");
  String payload;
    if (httpCode > 0) { //Check for the returning code
        payload = http.getString();
        
        Serial.println(httpCode);
        Serial.println(payload);
      }
    else {
      Serial.println("Error on HTTP request");
    }
  http.end();

 
}




//TFT LVGL Functions 
/********************************************************************************/

void guiTask(void *pvParameters) {
    
    InitTFTDisplay();

    uint16_t calData[5] = { 295, 3493, 320, 3602, 2 };
    tft.setTouch(calData);


    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * LV_VER_RES_MAX / 10);
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);             /*Descriptor of a input device driver*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;    /*Touch pad is a pointer-like device*/
    indev_drv.read_cb = my_touchpad_read;      /*Set your driver function*/
    lv_indev_drv_register(&indev_drv);         /*Finally register the driver*/
  
    lv_main();

  
    while (1) {
         lv_task_handler();
    }
}

/*
0, 334
1, 3439
2, 176
3, 3627
4, 2

*/

void InitTFTDisplay()
{
  lv_init();
  tft.begin(); /* TFT init */
  tft.setRotation(3); /* Landscape orientation */

  uint16_t calData[5] = { 334, 3439, 176,3627 , 1 };
  tft.setTouch(calData);

  lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

      /*Initialize the display*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    /*Initialize the (dummy) input device driver*/
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);             /*Descriptor of a input device driver*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;    /*Touch pad is a pointer-like device*/
    indev_drv.read_cb = my_touchpad_read;      /*Set your driver function*/
    lv_indev_drv_register(&indev_drv);         /*Finally register the driver*/

    lv_main();  // Create LVGL objects

  
    while (1) {
         lv_task_handler();
    }
    
}

int WriteMessageFlag = 0;

// Create LVGL Objects
static void lv_main(){
    
    //LV_THEME_MATERIAL_FLAG_LIGHT
    //LV_THEME_MATERIAL_FLAG_DARK
    
    lv_theme_t * th = lv_theme_material_init(LV_THEME_DEFAULT_COLOR_PRIMARY, LV_THEME_DEFAULT_COLOR_SECONDARY, LV_THEME_MATERIAL_FLAG_LIGHT, LV_THEME_DEFAULT_FONT_SMALL , LV_THEME_DEFAULT_FONT_NORMAL, LV_THEME_DEFAULT_FONT_SUBTITLE, LV_THEME_DEFAULT_FONT_TITLE);     
    lv_theme_set_act(th);

    lv_obj_t * scr = lv_obj_create(NULL, NULL);
    lv_scr_load(scr);

    static lv_style_t style1;
    lv_style_set_bg_color(&style1, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_clip_corner(&style1, LV_STATE_DEFAULT, false) ;
    lv_style_set_radius(&style1, LV_STATE_DEFAULT, 0);
    lv_style_set_border_width(&style1, LV_STATE_DEFAULT, 1);
    lv_style_set_border_color(&style1, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_border_color(&style1, LV_STATE_FOCUSED, LV_COLOR_RED);
    lv_style_set_outline_color(&style1, LV_STATE_FOCUSED, LV_COLOR_BLACK);
    lv_style_set_outline_width(&style1, LV_STATE_FOCUSED, 0);
    lv_style_set_outline_width(&style1, LV_STATE_DEFAULT, 0);
    // lv_style_set_shadow_color(&style1, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    // lv_style_set_shadow_width(&style1, LV_STATE_DEFAULT, 10);

    // lv_style_set_bg_color(&style1, LV_STATE_PRESSED, LV_COLOR_GRAY);
    // lv_style_set_bg_color(&style1, LV_STATE_FOCUSED, LV_COLOR_RED);
    // lv_style_set_bg_color(&style1, LV_STATE_FOCUSED | LV_STATE_PRESSED, lv_color_hex(0xf88));

    bg_left_panel = lv_obj_create(scr, NULL);
    lv_obj_clean_style_list(bg_left_panel, LV_OBJ_PART_MAIN);
    lv_obj_set_style_local_bg_opa(bg_left_panel, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,LV_OPA_COVER);
    lv_obj_set_style_local_bg_color(bg_left_panel, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,LV_COLOR_PINK);
    lv_obj_set_size(bg_left_panel, 50, LV_VER_RES);
    
    bg_right_panel = lv_obj_create(scr, NULL);
    lv_obj_clean_style_list(bg_right_panel, LV_OBJ_PART_MAIN);
    lv_obj_set_style_local_bg_opa(bg_right_panel, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,LV_OPA_COVER);
    lv_obj_set_style_local_bg_color(bg_right_panel, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,LV_COLOR_WHITE);
    lv_obj_set_pos(bg_right_panel, 50, 0);
    lv_obj_set_size(bg_right_panel, LV_HOR_RES, LV_VER_RES);

    // Write message button
    btn_write_message = lv_btn_create(bg_left_panel,NULL);
    lv_obj_set_event_cb(btn_write_message, BtnMessageWriteEventHandler);
    lv_obj_set_pos(btn_write_message, 0, 0);
    lv_obj_set_size(btn_write_message, 50, 50);
    lv_obj_add_style(btn_write_message, LV_BTN_PART_MAIN, &style1);  /*Overwrite only a some colors to red*/
    


    /* Create simple label */
    label = lv_label_create(bg_right_panel, NULL);
    lv_label_set_text(label, "Startup");
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);

    InitWriteMessageObjects();

 }

void InitWriteMessageObjects()
{

    /* Create the message box */
    ta_write_message = lv_textarea_create(bg_right_panel, NULL);
    lv_textarea_set_text(ta_write_message, "");
    //lv_textarea_set_pwd_mode(ta_write_message, true);
    //lv_textarea_set_one_line(ta_write_message, true);
    //lv_textarea_set_cursor_hidden(ta_write_message, true);
    lv_obj_set_size(ta_write_message, LV_HOR_RES - 50 - 20, (LV_VER_RES/2) -20 );
    lv_obj_set_pos(ta_write_message, 5, 10);
    lv_obj_set_event_cb(ta_write_message, ta_write_message_event);
    lv_obj_set_hidden(ta_write_message,true);

    // Create keyboard object  
    kb = lv_keyboard_create(bg_right_panel, NULL);
    lv_obj_set_size(kb,  LV_HOR_RES-50, LV_VER_RES / 2);

    lv_keyboard_set_textarea(kb, ta_write_message);
    lv_obj_set_event_cb(kb, keyboard_event_cb);
    lv_obj_set_hidden(kb,true);
  
}

static void ta_write_message_event(lv_obj_t * ta, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        /* Focus on the clicked text area */
        if(kb != NULL)
        lv_obj_set_hidden(kb,false);
        //lv_obj_move_foreground(kb); 
            lv_keyboard_set_textarea(kb, ta);
    }

    else if(event == LV_EVENT_INSERT) {
       // const char * str = lv_event_get_data();
       // if(str[0] == '\n') {
           // printf("Ready\n");
        //}
    }

}  

static void BtnMessageWriteEventHandler(lv_obj_t * btn_write_message, lv_event_t event)
{
      if(event == LV_EVENT_CLICKED) 
      {
        WriteMessageFlag = !WriteMessageFlag;
        if(WriteMessageFlag == true)
        {
          lv_obj_set_hidden(kb,false);
          lv_obj_set_hidden(ta_write_message,false);
        }
        else
        {
          lv_obj_set_hidden(kb,true);
          lv_obj_set_hidden(ta_write_message,true);
        }
        printf("Clicked\n");
      }
      lv_label_set_text(label, "Button Press");
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
}

// // Initialise the keyboard object
// static void makeKeyboard(){
//   kb = lv_keyboard_create(lv_scr_act(), NULL);
//   lv_obj_set_size(kb,  LV_HOR_RES, LV_VER_RES / 2);
//   lv_keyboard_set_cursor_manage(kb, true);
  
//   //lv_keyboard_set_textarea(kb, ta_password);
//   lv_obj_set_event_cb(kb, keyboard_event_cb);
//   lv_obj_move_background(kb);
// }

// Function to move the keyboard to the foreground when finished 
static void keyboard_event_cb(lv_obj_t * kb, lv_event_t event){
  lv_keyboard_def_event_cb(kb, event);
  
  if(event == LV_EVENT_APPLY){
    lv_obj_set_hidden(kb,true);
    Serial.println("Keyboard");
    
  }else if(event == LV_EVENT_CANCEL){
    lv_obj_set_hidden(kb,true);
    Serial.println("Keyboard1");
  }
}


/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    /* Create simple label */

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors(&color_p->full, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

/* Reading input device (simulated encoder here) */
bool read_encoder(lv_indev_drv_t * indev, lv_indev_data_t * data)
{
    static int32_t last_diff = 0;
    int32_t diff = 0; /* Dummy - no movement */
    int btn_state = LV_INDEV_STATE_REL; /* Dummy - no press */

    data->enc_diff = diff - last_diff;;
    data->state = btn_state;

    last_diff = diff;

    return false;
}

bool my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data)
{
    uint16_t touchX, touchY;

    bool touched = tft.getTouch(&touchX, &touchY, 600);

    if(!touched)
    {
      return false;
    }

    if(touchX > SCREEN_WIDTH || touchY > SCREEN_HEIGHT)
    {
      Serial.println("Y or y outside of expected parameters..");
      Serial.print("y:");
      Serial.print(touchX);
      Serial.print(" x:");
      Serial.print(touchY);
    }
    else
    {
      data->state = touched ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL; 
      data->point.x = touchX;
      data->point.y = touchY;
    }

    return false; /*Return `false` because we are not buffering and no more data to read*/
}





















