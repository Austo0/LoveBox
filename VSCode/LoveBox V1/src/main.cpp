#include <Arduino.h>
#include "main.h"

#include <Preferences.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Ping.h>
#include "time.h"
#include <Wire.h>
#include <SPI.h>
#include "ESP32TimerInterrupt.h"
// TFT Display Libraries 
#include <lvgl.h>
//#include <TFT_eSPI.h>

#include "LVGLDisplay.h"

/************************************************************/
// Functions
/************************************************************/
void GuiTask(void *pvParameters);
void ScanGSheetTask(void *pvParameters);
void HandleWifiConnection(void *pvParameters);
void PrintLocalTimeDate();
void WriteSpreadSheet();
String readSpreadSheet(byte modeType);
void ReadDataFromSheets();
void UpdateWifiStatusLabelText(String text);

/********************************************************************************/
//Things to change
// const char * ssid = "dlink-80EB";
// const char * password = "abc123abc";


String GOOGLE_SCRIPT_ID = "AKfycbwX8S-OX1MyQfS8jirMaF7FK2M1sBkYoVzDVRl3MpQibCIaqOGeq1TdCT8J6qhEY_oh";

const int sendInterval = 5000; 


String scanNetworkSSID = "";

int scanWifiFlag = 0;
int connectWifiFlag = 0;
int disconnectWifiFlag = 0;
String ssid = "dlink-80EB";
String password = "abc123abc";


const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 46800;
const int   daylightOffset_sec = 3600;

/********************************************************************************/

// Google Sheets Data Variables 
String dataDateString, dataTimeString, dataMessageString;
int dataSentFlag, dataReceivedFlag = 0;
int checkGoogleSheetsDataFlag = 1;



// Task Handlers
TaskHandle_t ntScanTaskHandler;
TaskHandle_t ntConnectTaskHandler;


// Initialise Instances
WiFiClientSecure client;

Preferences preferences;    // Instance of preferences for saving data to Flash

// Setup
void setup() 
{
  // Initialise serial port
  Serial.begin(115200);                       
  delay(10);

  //Create a namespace in preferences for wifi credentials
  preferences.begin("credentials", false);

  // Read wifi credentials from memory
  ssid = preferences.getString("ssid", ""); 
  password = preferences.getString("password", "");
  
// Set up RTOS tasks

// Create a task to run the TFT touch screen display
  xTaskCreate(
  GuiTask,            // Function that should be called
  "gui",              // Name of the task (for debugging)
  4096*2,             // Stack size (bytes)
  NULL,               // Parameter to pass
  2,                  // Task priority
  NULL                // Task handle
  );

  vTaskDelay(500);

// Create a task to run the gsheets data management
  xTaskCreate(
  ScanGSheetTask,    // Function that should be called
  "ScanGSheetTask",   // Name of the task (for debugging)
  4096*3,            // Stack size (bytes)
  NULL,            // Parameter to pass
  1,               // Task priority
  &ntScanTaskHandler             // Task handle
  );

  vTaskDelay(500);

// Create a task to handle the wifi connection
  xTaskCreate(
  HandleWifiConnection,    // Function that should be called
  "HandleWifiConnection",   // Name of the task (for debugging)
  4096*3,            // Stack size (bytes)
  NULL,            // Parameter to pass
  1,               // Task priority
  &ntScanTaskHandler             // Task handle
  );

}

// Connect to wifi
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

      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      PrintLocalTimeDate();
    }    
    else
    {
      Serial.println("WIFI Connection Failed");
      UpdateWifiStatusLabelText("Failed");
    }      
    Serial.println(WiFi.localIP());  
  }
}

// Disconnect from wifi
void DisconnectWifi()
{
  WiFi.disconnect();
}


// Main loop
void loop() 
{

}



void HandleWifiConnection(void *pvParameters)
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
        UpdateWifiScanDropDown(scanNetworkSSID);
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

void ScanGSheetTask(void *pvParameters) 
{ 

  while(1)
  {
 
    vTaskDelay(10000);
    if(WiFi.status() == WL_CONNECTED)
    {
       //ReadDataFromSheets();  
       PrintLocalTimeDate();
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

void WriteSpreadSheet()
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




void PrintLocalTimeDate()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
    // Serial.println(&timeinfo, "%I");
  // Serial.print("Minute: ");
  // Serial.println(&timeinfo, "%M");
  // Serial.print("Second: ");
  char timeHourMin[6];
  char meridiem[3];
  char timeHourMinMeridiem[10];
  strftime(timeHourMin,6, "%I:%M", &timeinfo);
strftime(meridiem,3, "%p", &timeinfo);
 sprintf(timeHourMinMeridiem, "%s %s", timeHourMin, meridiem);
 UpdateTimeDateLabelText(timeHourMinMeridiem);
  Serial.println(timeHourMinMeridiem);

  // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  // Serial.print("Day of week: ");
  // Serial.println(&timeinfo, "%A");
  // Serial.print("Month: ");
  // Serial.println(&timeinfo, "%B");
  // Serial.print("Day of Month: ");
  // Serial.println(&timeinfo, "%d");
  // Serial.print("Year: ");
  // Serial.println(&timeinfo, "%Y");
  // Serial.print("Hour: ");
  // Serial.println(&timeinfo, "%H");
  // Serial.print("Hour (12 hour format): ");
  // Serial.println(&timeinfo, "%I");
  // Serial.print("Minute: ");
  // Serial.println(&timeinfo, "%M");
  // Serial.print("Second: ");
  // Serial.println(&timeinfo, "%S");

  // Serial.println("Time variables");

  // Serial.println(timeHour);
  // char timeWeekDay[10];
  // strftime(timeWeekDay,10, "%A", &timeinfo);
  // Serial.println(timeWeekDay);
  // Serial.println();
}














