/*	Project : Read Google Spread Sheet Data from ESP32	*/
/*Refer following video for complete project : https://youtu.be/0LoeaewIAdY*/


#include <WiFi.h>
#include <HTTPClient.h>

#include <Wire.h>
#include <SPI.h>

// TFT Display Libraries 
#include <lvgl.h>
#include <TFT_eSPI.h>

// Definitions
#define MODE_READ_CELL_RANGE 0
#define MODE_READ_LAST_CELL 1


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
static lv_obj_t *label;

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
  InitTFTDisplay();



  // Initialise Timer0 interrupt
  ITimer0.attachInterruptInterval(TIMER0_INTERVAL_MS * 1000, TimerHandler0);


}

// Interrupt every TIMER0_INTERVAL_MS to check for updated data from google sheets
void TimerHandler0()
{
  checkGoogleSheetsDataFlag = 1;
}

// Main loop
void loop() 
{
  if(checkGoogleSheetsDataFlag == 1)
  {
    ReadDataFromSheets();
    checkGoogleSheetsDataFlag = 0;

    lv_label_set_text(label, dataMessageString.c_str());
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
    
  }
  
    lv_task_handler(); /* let the GUI do its work */
    delay(5);
}



// Google Sheets Data Functions
/********************************************************************************/
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

void InitTFTDisplay()
{
  lv_init();
  tft.begin(); /* TFT init */
  tft.setRotation(3); /* Landscape orientation */

  lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

      /*Initialize the display*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 320;
    disp_drv.ver_res = 240;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    /*Initialize the (dummy) input device driver*/
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_ENCODER;
    indev_drv.read_cb = read_encoder;
    lv_indev_drv_register(&indev_drv);

    /* Create simple label */
    label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(label, "Startup");
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * kb = lv_keyboard_create(lv_scr_act());
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);

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























