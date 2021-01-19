/* 
## IoTAlarm ##

A simple Clock Alarm built-on arduino-like microcontroller called NodeMCU that based on ESP8266 WiFi chip.
It may be regarded as a replacement for the traditional clock alarms.
As name implies, it tends to make an action upon running the alarm.
Actions may be like toggling a relay, running a motor or even sending any kind of signal/message at the abstarct level.


By: Osama Zidan
Jan, 19 2021
*/

#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h> 

#define MAIN_LOOP_DELAY 100
#define LONG_DEEP_SLEEP_DELAY 3600 // 1 hour
// IMPORTANT:
//  We treat timestamp in form of total seconds
#define TIMESTAMP_MAX_DIFF 3600   //  secs
#define TIMESTAMP_MIN_DIFF 13     //  secs

void connect();
bool CheckEEPROM();
void ReadEEPROM();
void WriteEEPROM();
void setAlarm();
void stopAlarm();
void runAlarm();


struct Time{
  uint8_t h;
  uint8_t m;
  uint8_t s;
  bool active;
  // return number of total seconds
  int32_t getTimestamp(){
    return h * 60 * 60 + m * 60 + s;
  }
};


const char *ssid     = "************";
const char *password = "************";

const long utcOffsetInSeconds = 7200; // utc+2

// modes in which the code operates
enum MODE{NORMAL=0, SET_ALARM=1, STOP_ALARM=2, RUN_ALARM=3, IDLE };
volatile uint8_t setMode = NORMAL;

// initial alarm time if it isn't set yet
int8_t ALARM_H = 21; 
int8_t ALARM_M = 19; 
int8_t ALARM_S = 0;
byte active = 0;

// hold the difference between timestamps of "alarm" and "last_fetched_time"
volatile int32_t diff_timestamp;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// create Time object (last_fetched_time)
Time *last_fetched_time = new Time;

// create Time object (alarm_time)
Time *alarm = new Time;

//////////////////////////////////////////////////////
/* SET UP SECTION */
void setup(){
  Serial.begin(115200);

  // connect to WiFi
  //  connect();
  timeClient.begin();
  
  // set inital alarm time which is stored in EEPROM
  if (CheckEEPROM()){
    Serial.println("Checking EEPROM...");
    setAlarm();
  }

}

/////////////////////////////////////////////////////

void loop() {
  // check mode
  switch(setMode){
    case NORMAL:
      Serial.println("Normal mode.");
      // wait to re-connect to WiFi
      if(WiFi.status() != WL_CONNECTED){
        connect();
        timeClient.begin();
      }
      //fetch current time from ntp protocol
      timeClient.update();
      
      // push the output to "last_fetched_time" struct
      last_fetched_time->h = timeClient.getHours();
      last_fetched_time->m = timeClient.getMinutes();
      last_fetched_time->s = timeClient.getSeconds();

      /* DEBUGING */
      Serial.print("lft   = ");
      Serial.print(last_fetched_time->h);
      Serial.print(" : ");
      Serial.print(last_fetched_time->m);
      Serial.print(" : ");
      Serial.println(last_fetched_time->s);

      Serial.print("alarm = ");
      Serial.print(alarm->h);
      Serial.print(" : ");
      Serial.print(alarm->m);
      Serial.print(" : ");
      Serial.println(alarm->s);

      Serial.print("lft_timestamp  = ");
      Serial.println(last_fetched_time->getTimestamp());

      Serial.print("alarm_timestamp= ");
      Serial.println(alarm->getTimestamp());

      // if alarm isn't due yet, deep sleep to save power consumption
      // sleep every certain time + 2 minutes as tolerance
      diff_timestamp = alarm->getTimestamp() - last_fetched_time->getTimestamp();
      Serial.print("diff_timestamp = ");
      Serial.println(diff_timestamp);

      if ( diff_timestamp > (TIMESTAMP_MAX_DIFF +2) ){
        // gonna sleep for a while
        Serial.println("Long DEEP SLEEP");
        // deepSleep() expects number of microsenconds
        ESP.deepSleep(LONG_DEEP_SLEEP_DELAY * 1e6); // 1e6 = 1 x 10^6
      }else if (diff_timestamp > TIMESTAMP_MIN_DIFF){
        // sleep for the remaining time
        // subtract "TIMESTAMP_MIN_DIFF" minutes for tolerance, then convert to seconds
        Serial.println("Light DEEP SLEEP");
        ESP.deepSleep((diff_timestamp - TIMESTAMP_MIN_DIFF) * 1e6); 
      }else if (diff_timestamp <= 0) {
        setMode = IDLE;
      }

      // check if alarm is due
      if( alarm->active ){
        if((alarm->h == last_fetched_time->h) && (alarm->m == last_fetched_time->m) && (alarm->s >= 0 && alarm->s <= 3)){
          // go to RUN_ALARM mode to start the alarm
          setMode = RUN_ALARM;
        }
      }
      break;

    case SET_ALARM:
      Serial.println("Setting alarm...");
      setAlarm();
      setMode = NORMAL;
      break;

    case STOP_ALARM:
      Serial.println("Stoping Alarm...");
      stopAlarm();
      setMode = NORMAL;
      break;

    case RUN_ALARM:
      runAlarm();
      setMode = IDLE;
      break;

    case IDLE:
      setMode = NORMAL;
      ESP.deepSleep(LONG_DEEP_SLEEP_DELAY * 1e6);
      break;
  }
   delay(MAIN_LOOP_DELAY);
}

void connect() {

  // Connect to Wifi.
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // WiFi fix: https://github.com/esp8266/Arduino/issues/2186
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long wifiConnectStart = millis();

  while (WiFi.status() != WL_CONNECTED) {
    // Check to see if
    if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.println("Failed to connect to WiFi. Please verify credentials: ");
      delay(10000);
    }

    delay(500);
    Serial.println("...");
    // Only try for 5 seconds.
    if (millis() - wifiConnectStart > 15000) {
      Serial.println("Failed to connect to WiFi");
      return;
    }
  }
}


void setAlarm(){
  alarm->active = 1;
  alarm->h = ALARM_H;
  alarm->m = ALARM_M;
  alarm->s = ALARM_S;

  WriteEEPROM();
  Serial.println("Setting Alarm Done!");
}


void stopAlarm(){
  alarm->active = 0;
  WriteEEPROM();
  Serial.println("Stoping Alarm Done!");
}

void runAlarm(){
  Serial.println("Alarm is due! Ring Ring.");
  delay(2000);
}

void ReadEEPROM () {
  alarm->h = EEPROM.read(1);
  alarm->m = EEPROM.read(2);
  alarm->s = EEPROM.read(3);
  
  alarm->active = EEPROM.read(4); 
}

void WriteEEPROM () {
  EEPROM.write(1,alarm->h);
  EEPROM.write(2,alarm->m);
  EEPROM.write(3,alarm->s);
  
  EEPROM.write(4,alarm->active);
}

bool CheckEEPROM(){
  // Memory is never “empty” it always has something in it.
  // just check if stored value are valid for our scenario
  // return:
  // true  -> valid values  
  // false -> at least one invalid value saved in the first four addresses of EEPROM

  // check "hours" value
  if(EEPROM.read(1) > 23){
    return false;
  }

  for(int8_t i=1; i < 3; i++){
    // upper range value is 60 (minutes & seconds)
    if(EEPROM.read(i) > 60)
      return false;
  }

  if(EEPROM.read(4) > 1)
    return false;

  return true;
}
