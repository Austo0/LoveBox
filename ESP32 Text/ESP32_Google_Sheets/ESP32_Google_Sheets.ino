/*	Project : Read Google Spread Sheet Data from ESP32	*/
/*Refer following video for complete project : https://youtu.be/0LoeaewIAdY*/


#include <WiFi.h>
#include <HTTPClient.h>

#include <Wire.h>


/********************************************************************************/
 
#include <SPI.h>

//Things to change
const char * ssid = "dlink-80EB";
const char * password = "abc123abc";
String GOOGLE_SCRIPT_ID = "AKfycbwX8S-OX1MyQfS8jirMaF7FK2M1sBkYoVzDVRl3MpQibCIaqOGeq1TdCT8J6qhEY_oh";

const int sendInterval = 5000; 
/********************************************************************************/
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels


#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

#define MODE_READ_CELL_RANGE 0
#define MODE_READ_LAST_CELL 1

WiFiClientSecure client;


String dataDateString, dataTimeString, dataMessageString;
int dataSentFlag, dataReceivedFlag = 0;

void setup() {
  Serial.begin(115200);
  delay(10);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println("Started");
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
Serial.println("Ready to go");
//testdrawstyles();
}




void loop() 
{
  ReadDataFromSheets();
  delay(sendInterval);
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
   //sscanf(spreadSheetReturnString.c_str(),"%s,%s,$s,%i,%i", dataDateString, dataTimeString, dataMessageString, dataSentFlag, dataReceivedFlag);
   sprintf(parsedData,"Date: %s, Time: %s, Message: %s, Sent Flag: %i, Received Flag: %i", dataDateString, dataTimeString, dataMessageString, dataSentFlag, dataReceivedFlag);
  // Serial.println(dataDateString);
  // Serial.println(dataTimeString);
  // Serial.println(dataMessageString);
  // Serial.println(dataSentFlag);
  // Serial.println(dataReceivedFlag);
   
  // Serial.println(parsedData);
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
void spreadsheet_comm(void) {

}
