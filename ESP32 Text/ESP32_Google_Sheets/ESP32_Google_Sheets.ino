/*	Project : Read Google Spread Sheet Data from ESP32	*/
/*Refer following video for complete project : https://youtu.be/0LoeaewIAdY*/

#include <Preferences.h>
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

//TFT LVGL Objects
static lv_obj_t * scr;                // Main Screen object

void UpdateWifiStatusLabelText(String text);

//Screen objects
static lv_obj_t * scr_home;
static lv_obj_t * scr_settings;


// Home Screen

// Settings Screen
static lv_obj_t *label_settings_WIFI_status;
static lv_obj_t *ta_ssid;
static lv_obj_t *ta_password;
static lv_obj_t * wifi_scan_list;

// Interrupt Timer Definitions
#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     0
#include "ESP32TimerInterrupt.h"
#define TIMER0_INTERVAL_MS        20000
void IRAM_ATTR TimerHandler0(void);

/********************************************************************************/
//Things to change
// const char * ssid = "dlink-80EB";
// const char * password = "abc123abc";
String ssid = "dlink-80EB";
String password = "abc123abc";

String GOOGLE_SCRIPT_ID = "AKfycbwX8S-OX1MyQfS8jirMaF7FK2M1sBkYoVzDVRl3MpQibCIaqOGeq1TdCT8J6qhEY_oh";

const int sendInterval = 5000; 

int scanWifiFlag = 0;
int connectWifiFlag = 0;
int disconnectWifiFlag = 0;
String scanNetworkSSID = "";
/********************************************************************************/

// Google Sheets Data Variables 
String dataDateString, dataTimeString, dataMessageString;
int dataSentFlag, dataReceivedFlag = 0;
int checkGoogleSheetsDataFlag = 1;

// TFT LVGL Variables
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];




TaskHandle_t ntScanTaskHandler;
TaskHandle_t ntConnectTaskHandler;

// Timer Interrupt Variables
volatile uint32_t Timer0Count = 0;
ESP32Timer ITimer0(0);

// Initialise Instances
WiFiClientSecure client;
TFT_eSPI tft = TFT_eSPI();    /* TFT instance */
Preferences preferences;    // Instance of preferences for saving data to Flash

// Setup
void setup() 
{
  // Initialise serial port
  Serial.begin(115200);                       
  delay(10);

  //Create a namespace in preferences for wifi credentials
  preferences.begin("credentials", false);

  ssid = preferences.getString("ssid", ""); 
  password = preferences.getString("password", "");
  
 
 
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

  vTaskDelay(500);

    xTaskCreate(
    handleWifiConnection,    // Function that should be called
    "HandleWifiConnection",   // Name of the task (for debugging)
    4096*3,            // Stack size (bytes)
    NULL,            // Parameter to pass
    1,               // Task priority
    &ntScanTaskHandler             // Task handle
  );




  // Initialise Timer0 interrupt
  //ITimer0.attachInterruptInterval(TIMER0_INTERVAL_MS * 1000, TimerHandler0);


}


void ConnectToWifi()
{
  if (ssid == "" || password == "")
  {
    Serial.println("No values saved for ssid or password");
  }
  else 
  {
    // Connect to Wi-Fi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.print("Connecting to WiFi ..");
    
    int i = 0;
    while((!WiFi.status() || WiFi.status() >= WL_DISCONNECTED) && i < 100) {
        delay(100);
        i++;
    }
    
    if(WiFi.status() == WL_CONNECTED)
    {
      Serial.println("WIFI Connected");
      UpdateWifiStatusLabelText("Connected");
      preferences.putString("ssid", ssid); 
      preferences.putString("password", password);
    }    
    else
    {
      Serial.println("WIFI Connection Failed");
      UpdateWifiStatusLabelText("Failed");
    }      
    Serial.println(WiFi.localIP());  
  }
}

void DisconnectWifi()
{
  WiFi.disconnect();
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

void handleWifiConnection(void *pvParameters)
{
  vTaskDelay(1000);
  ConnectToWifi();
  while(1)
  {
    vTaskDelay(100);
    if(scanWifiFlag == 1)
    {
      scanWifiFlag = 0;
      int n = WiFi.scanNetworks();
      scanNetworkSSID = "";
      if (n == 0) 
      {
        scanNetworkSSID += "No networks found";
      } 
      else 
      {
        for (int i = 0; i < n; ++i) 
        {
          // Print SSID and RSSI for each network found
          scanNetworkSSID += WiFi.SSID(i);
          if(i != (n-1))
          {
            scanNetworkSSID += "\n"; 
          }          
          
        }
        wifi_scan_list = lv_dropdown_create(scr_settings, NULL);
        lv_obj_set_pos(wifi_scan_list, 145, 20); 
        lv_dropdown_set_options(wifi_scan_list,scanNetworkSSID.c_str());
        lv_obj_set_event_cb(wifi_scan_list, WifiScanDropdownEventHandler);
        Serial.print(scanNetworkSSID);
      }
    }

  if(WiFi.status() != WL_CONNECTED)
  {
    UpdateWifiStatusLabelText("Disconnected");
  }  

  if(connectWifiFlag == 1)
  {
    connectWifiFlag = 0;
    ConnectToWifi();    
  }  

  if(disconnectWifiFlag == 1)
  {
    disconnectWifiFlag = 0;
    DisconnectWifi();
  }
  
  }
}

// Google Sheets Data Functions
/********************************************************************************/

void scanGSheetTask(void *pvParameters) 
{ 

  while(1)
  {
 
    vTaskDelay(10000);
    if(WiFi.status() == WL_CONNECTED)
    {
       ReadDataFromSheets();  
    }
   
    // lv_label_set_text(label, dataMessageString.c_str());
    // lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
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






















