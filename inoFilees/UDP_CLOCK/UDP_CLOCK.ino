/// @file    Blink.ino
/// @brief   Blink the first LED of an LED strip
/// @example Blink.ino

#include <FastLED.h>
#include <vector>
#include <map>
#include <string> 
#include <unordered_map>
#include <iomanip>
#include <cmath>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

#pragma region inits
//wifi
// WiFi network name and password:
const char * ssid = "robobrawlbutton";
const char * password = "doyourobotics";

//IP address to send UDP data to:
// either use the ip address of the server or 
// a network broadcast address
const char * udpAddress = "224.1.1.1";
const int udpSend = 5006;
const int udpRecieve = 5007;

boolean connected = false;

// coppy vars from message struct
bool runclock = 0;
bool orangeTapout = 0;
bool blueTapout = 0;
bool reset = 0;

bool jason = 1;

WiFiUDP udp;


// How many leds in your strip?
#define NUM_LEDS 95 

// For led chips like WS2812, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
// Clock pin only needed for SPI based chipsets when not using hardware SPI
#define DATA_PIN 13 //D1 //13 on ESP
#define CLOCK_PIN 22 // D5 //22 on ESP

std::map<std::string, std::vector<int>> TEST_MAP; 

struct RecData
{
  bool startClock;
  bool reset;
  bool pause;
  int  win;
  bool readyBlue;
  bool readyOrange;
  bool orangeTapin;
  bool blueTapin;
  /* data */
};

struct SendData
{
  bool run;
  bool reset_ack;
  bool done;
  /* data */
};

RecData recData;
SendData sendData;

#pragma endregion inits

void initMap() {
                      // 6, 1, 2, 4, 5, 7, 3
  TEST_MAP["dig1"] = {20, 5, 8, 14, 17, 23, 11};
  TEST_MAP["dig2"] = {41, 26, 29, 35, 38, 44, 32};
  TEST_MAP["dig3"] = {65, 50, 53, 59, 62, 68, 56};
  TEST_MAP["dig4"] = {86, 71, 74, 80, 83, 89, 77};
  TEST_MAP["dot1"] = {48}; //upper colon
  TEST_MAP["dot2"] = {47}; //lower colon
  TEST_MAP["dot3"] = {46}; //period 
  TEST_MAP["gear1"] = {0, 1, 2, 3};
  TEST_MAP["gear2"] = {92, 93, 94, 95};
}

void connectToWiFi(const char * ssid, const char * pwd){
  Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);
  
  //Initiate connection
  WiFi.begin(ssid, pwd);

  Serial.println("Waiting for WIFI connection...");
}

//wifi event handler
void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);

  switch (event) {
    case ARDUINO_EVENT_WIFI_READY:               Serial.println("WiFi interface ready"); break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:           Serial.println("Completed scan for access points"); break;
    case ARDUINO_EVENT_WIFI_STA_START:           Serial.println("WiFi client started"); break;
    case ARDUINO_EVENT_WIFI_STA_STOP:            Serial.println("WiFi clients stopped"); break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:       Serial.println("Connected to access point"); break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:    Serial.println("Disconnected from WiFi access point"); break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE: Serial.println("Authentication mode of access point has changed"); break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("Obtained IP address: ");
      Serial.println(WiFi.localIP());
      //udp.beginMulticast(IPAddress(244, 1, 1, 1), udpRecieve);
      break;
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:        Serial.println("Lost IP address and IP address is reset to 0"); break;
    case ARDUINO_EVENT_WPS_ER_SUCCESS:          Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode"); break;
    case ARDUINO_EVENT_WPS_ER_FAILED:           Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode"); break;
    case ARDUINO_EVENT_WPS_ER_TIMEOUT:          Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode"); break;
    case ARDUINO_EVENT_WPS_ER_PIN:              Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode"); break;
    case ARDUINO_EVENT_WIFI_AP_START:           Serial.println("WiFi access point started"); break;
    case ARDUINO_EVENT_WIFI_AP_STOP:            Serial.println("WiFi access point  stopped"); break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:    Serial.println("Client connected"); break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED: Serial.println("Client disconnected"); break;
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:   Serial.println("Assigned IP address to client"); break;
    case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:  Serial.println("Received probe request"); break;
    case ARDUINO_EVENT_WIFI_AP_GOT_IP6:         Serial.println("AP IPv6 is preferred"); break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:        Serial.println("STA IPv6 is preferred"); break;
    case ARDUINO_EVENT_ETH_GOT_IP6:             Serial.println("Ethernet IPv6 is preferred"); break;
    case ARDUINO_EVENT_ETH_START:               Serial.println("Ethernet started"); break;
    case ARDUINO_EVENT_ETH_STOP:                Serial.println("Ethernet stopped"); break;
    case ARDUINO_EVENT_ETH_CONNECTED:           Serial.println("Ethernet connected"); break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:        Serial.println("Ethernet disconnected"); break;
    case ARDUINO_EVENT_ETH_GOT_IP:              Serial.println("Obtained IP address"); break;
    default:                                    break;
  }
}

// WARNING: This function is called from a separate FreeRTOS task (thread)!
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));
}


void printHi() {
  std::vector<bool> letterH = {false, true, false, true, true, true, true}; 
  std::vector<bool> letterI = {false, false, false, false, false, false, true}; 

  changeNumLed(letterH, "dig3", 100);
  changeNumLed(letterI, "dig2", 100); 
  delay(3000); 
  FastLED.show(); 
  delay(1); 
}

void printJSON() {
  std::vector<bool> letterJ = {false, true, true, false, true, false, true}; 
  std::vector<bool> letterS = {true, true, true, true, false, true, false}; 
  std::vector<bool> letterO = {true, true, true, false, true, true, true}; 
  std::vector<bool> letterN = {true, true, false, false, true, true, true}; 

  changeNumLed(letterJ, "dig4", 100);
  changeNumLed(letterS, "dig3", 100); 
  changeNumLed(letterO, "dig2", 100);
  changeNumLed(letterN, "dig1", 100); 
  delay(3000); 
  FastLED.show(); 
  delay(1); 
}

void printSLUGS() {
  std::vector<bool> letterS = {true, true, true, true, false, true, false}; 
  std::vector<bool> letterL = {false, false, true, false, false, true, true}; 
  std::vector<bool> letterU = {false, true, true, false, true, true, true}; 
  std::vector<bool> letterG = {true, true, true, true, false, true, true};  

  changeNumLed(letterS, "dig4", 100);
  changeNumLed(letterL, "dig3", 100); 
  changeNumLed(letterU, "dig2", 100);
  changeNumLed(letterG, "dig1", 100); 
  delay(3000); 
}

void print_Jd_() {
  std::vector<bool> letterDash = {true, true, true, true, false, true, false}; 
  std::vector<bool> letterJ = {false, true, true, false, true, false, true}; 
  std::vector<bool> letterd = {false, true, true, false, true, true, true}; //fix

  changeNumLed(letterDash, "dig4", 100);
  changeNumLed(letterJ, "dig3", 100); 
  changeNumLed(letterd, "dig2", 100);
  changeNumLed(letterDash, "dig1", 100); 
  delay(3000); 
  FastLED.show(); 
  delay(1); 
}

// Define the array of leds
CRGB leds[NUM_LEDS];

void setup() { 

  Serial.begin(115200);

  // delete old config
  WiFi.disconnect(true);

  delay(1000);

  // Examples of different ways to register wifi events;
  // these handlers will be called from another thread.
  WiFi.onEvent(WiFiEvent);
  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFiEventId_t eventID = WiFi.onEvent(
    [](WiFiEvent_t event, WiFiEventInfo_t info) {
      Serial.print("WiFi lost connection. Reason: ");
      Serial.println(info.wifi_sta_disconnected.reason);
    },
    WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED
  );

  // Remove WiFi event
  Serial.print("WiFi Event ID: ");
  Serial.println(eventID);
  // WiFi.removeEvent(eventID);

  WiFi.begin(ssid, password);

  Serial.println();
  Serial.println();
  Serial.println("Wait for WiFi... ");

  udp.beginMulticast(IPAddress(224, 1, 1, 1), 5007);

  initMap(); 
  Serial.println("Init"); 
  FastLED.addLeds<WS2811, DATA_PIN, BRG>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();


}
#pragma region clock_test

void test_1() {
  Serial.println("Blink"); 
  leds[0] = CRGB(120, 255, 0); 
  leds[1] = CRGB::Red; 
  FastLED.show();
  delay(500);
  // Now turn the LED off, then pause
  leds[0] = CRGB::Black;
  leds[1] = CRGB::Black;
  FastLED.show();
  delay(500);
  Serial.println("UnBlink"); 
}

void test_2(int num_lds) {
  for (int i = 0; i < num_lds; i++) {
    leds[i] = CRGB::Red; 
    FastLED.show();
    delay(250);
    leds[i] = CRGB::Black; 
    FastLED.show();
    delay(250);
  }
}

// PINK: 190, 30, 30
// snake design
void test_4() {
  for (unsigned i = 0; i < NUM_LEDS-1; i++) {
    FastLED.clear();
    if (i == 0) {
      leds[0] = CRGB::Green;
      Serial.println("0: Green"); 
    }
    else {
      leds[i-1] = CRGB::Red;  
      leds[i] = CRGB::Green; 
      // Serial.print("Red: ");
      // Serial.print(i-1); 
      // Serial.print(", Green: "); 
      Serial.println(i); 
      FastLED.show();
      delay(350);
      // leds[i] = CRGB(120, 255, 0); 
      // FastLED.show();
      // delay(250);
    }
  }
}

void test_3() {
  int r = 0; 
  int g = 0; 
  int b = 0; 

  for (unsigned i = 0; i < NUM_LEDS; i++) {
    int r = r + 20; 
    int g = g + 10; 
    int b = b + 5; 
    leds[i] = CRGB(r % 255, g % 255, b % 255); 
    FastLED.show();
    delay(250);
    // leds[i] = CRGB(120, 255, 0); 
    FastLED.show();
    delay(250);
  }
}


std::unordered_map<int, std::vector<bool>> sevenSegmentMap = {

    {0, {true, true, true, false, true, true, true}},
    {1, {false, true, false, false, true, false, false}},
    {2, {true, false, true, true, true, false, true}},
    {3, {true, true, true, true, true, false, false}},
    {4, {false, true, false, true, true, true, false}},
    {5, {true, true, true, true, false, true, false}},
    {6, {true, true, true, true, false, true, true}},
    {7, {true, true, false, false, true, false, false}},
    {8, {true, true, true, true, true, true, true}},
    {9, {true, true, true, true, true, true, false}}
};



std::vector<bool> getSevenSegmentDisplay(int digit) {
    return sevenSegmentMap[digit];
}

void lightZeroWinnerColor(std::string winner) {
  changeNumLedWC(getSevenSegmentDisplay(0), "dig1", winner); 
  changeNumLedWC(getSevenSegmentDisplay(0), "dig2", winner); 
  changeNumLedWC(getSevenSegmentDisplay(0), "dig3", winner); 
  changeNumLedWC(getSevenSegmentDisplay(0), "dig4", winner); 
  FastLED.show(); 
  delay(1); 
}

struct CRGB getWinnerColor(std::string winner) {
  if (winner == "blue") {
    return CRGB::Blue; 
  }
  if (winner == "orange") {
    return CRGB::Orange; 
  }
}

std::vector<int> secondsToMinuteDigits(int seconds) {
    int minutes = seconds / 60;
    int remainingSeconds = (seconds % 60);
    
    return {
        minutes / 10,  // Tens place of minutes
        minutes % 10,  // Ones place of minutes
        remainingSeconds / 10,  // Tens place of seconds
        remainingSeconds % 10   // Ones place of seconds
    };
}
// changes color of board
struct CRGB getColor(float seconds) {
  if (seconds >= 30) {
    return CRGB::Green; 
  }
  else if (seconds >= 10) {
    return CRGB::Yellow; 
  }
  else {
    return CRGB::Red; 
  }
} 

void goToBlack() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black; 
  }
  FastLED.show(); 
  delay(1); 
}

void setToColor(struct CRGB color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    if (leds[i] != CRGB::Black) {
      leds[i] = color; 
    }
  }
  FastLED.show(); 
  delay(1); 
}

void setAlltoColor(struct CRGB color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color; 
  }
  FastLED.show(); 
  delay(1); 
}



std::vector<int> formatTime(double inputTime) {
    std::vector<int> result(5, 0);  // Initialize vector with five zeros
    // Extract minutes and seconds
    int totalSeconds = static_cast<int>(inputTime);
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    // Extract tenths of a second
    int tenths = static_cast<int>(floor((inputTime - totalSeconds) * 10));
    // Determine separator: colon (1) for minutes:seconds, decimal (0) for seconds.tenths
    Serial.println("I THINK ITS AROUND HEREEEEEEEEEEEEEEEEEEEEEEEEEEE");

    bool useColon = (totalSeconds >= 10);
    if (useColon) {
        result[0] = minutes / 10;      // First digit of minutes
        result[1] = minutes % 10;      // Second digit of minutes
        result[2] = 1;                 // Separator (colon)
        result[3] = seconds / 10;      // First digit of seconds
        result[4] = seconds % 10;      // Second digit of seconds
    } else {
        result[0] = seconds / 10;      // First digit of seconds
        result[1] = seconds % 10;      // Second digit of seconds
        result[2] = 0;                 // Separator (decimal)
        result[3] = tenths;            // Tenths of a second
        result[4] = 0;                 // Placeholder zero
    }

    return result;
}


std::string formatBoolVector(const std::vector<bool>& vec) {
    std::string result = "{";
    
    for (size_t i = 0; i < vec.size(); i++) {
        result += vec[i] ? "true" : "false";
        if (i < vec.size() - 1) {
            result += ", ";
        }
    }
    
    result += "}";
    return result;
}

std::string formatIntVector(const std::vector<int>& vec) {
    std::string result = "{";
    
    for (size_t i = 0; i < vec.size(); i++) {
        result += std::to_string(vec[i]);
        if (i < vec.size() - 1) {
            result += ", ";
        }
    }
    
    result += "}";
    return result;
}


void changeNumLed(std::vector<bool> ledVec, std::string name, float seconds) {
  for (int i = 0; i < 7; i++) {
    if (ledVec[i]) {
      leds[TEST_MAP[name][i]-1] = getColor(seconds);
      leds[TEST_MAP[name][i]] = getColor(seconds);
      leds[TEST_MAP[name][i]+1] = getColor(seconds); 
    }
    else {
      leds[TEST_MAP[name][i]-1] = CRGB::Black;
      leds[TEST_MAP[name][i]] = CRGB::Black;
      leds[TEST_MAP[name][i]+1] = CRGB::Black; 
    }

    FastLED.show(); 
    delay(1); 
  }
}

void changeNumLedWC(std::vector<bool> ledVec, std::string name, std::string winner) {
  for (int i = 0; i < 7; i++) {
    if (ledVec[i]) {
      leds[TEST_MAP[name][i]-1] = getWinnerColor(winner); 
      leds[TEST_MAP[name][i]] = getWinnerColor(winner); 
      leds[TEST_MAP[name][i]+1] = getWinnerColor(winner); 
    }
    else {
      leds[TEST_MAP[name][i]-1] = CRGB::Black;
      leds[TEST_MAP[name][i]] = CRGB::Black;
      leds[TEST_MAP[name][i]+1] = CRGB::Black; 
    }

    FastLED.show(); 
    delay(1); 
  }

}

void testDigitInt() {
  // std::string name = "dig1"; 
  // int displayNum = 5; 
  std::vector<std::string> names = {"dig1", "dig2", "dig3", "dig4"}; 

  // changeNumLed(getSevenSegmentDisplay(displayNum), name, 100); 

  for (unsigned i = 0; i < 10; i++) {
    for (unsigned j = 0; j < 4; j++) {
      changeNumLed(getSevenSegmentDisplay(i), names[j], 100); 
      FastLED.show(); 
      delay(2000); 
    }
    
  }
  FastLED.show(); 
  delay(1); 
}


void testNumber() {
  std::string name = "dig4"; 
  for (int i = 0; i < TEST_MAP[name].size(); i++) {
    leds[TEST_MAP[name][i]-1] = getColor(100);
    leds[TEST_MAP[name][i]] = getColor(100);
    leds[TEST_MAP[name][i]+1] = getColor(100); 
    
  }
  FastLED.show(); 
  delay(1); 
}
#pragma endregion clock_stuff


// just making there here so I dont have to scroll
bool setStartClock = false;
float clockStartTime = 0;
float sec;

int testIntegration(float seconds, bool pause) {
  if(! setStartClock){
    clockStartTime = millis();
    setStartClock = 1;
  }

  if( pause ) {
    Serial.println((sec * 1000));
    clockStartTime = millis() + ((sec - seconds) * 1000);
    return 1; 
  }

  sec = seconds - ((millis() - clockStartTime) / 1000);
  if(sec <= 0 ){
      runclock = 0;
      reset = 1; 
      setStartClock = 0;
      return 0;
    }

  Serial.println(sec);
  //for (float sec = seconds; sec >0 ; sec = sec - 0.1) {
    //Serial.print("SECONDS: "); 
    //Serial.print(sec); 
    //Serial.print(": "); 
    //delay(1); 
    std::vector<int> vecTime = formatTime(sec); 
    Serial.println(formatIntVector(vecTime).c_str()); 
    // 120: 02:00 -> {0, 2, 1, 0, 0}, 9.50: 09.50 -> {0, 9, 0, 5, 0}
    std::vector<bool> num1leds = getSevenSegmentDisplay(vecTime[0]); 
    std::vector<bool> num2leds = getSevenSegmentDisplay(vecTime[1]); 
    std::vector<bool> num3leds = getSevenSegmentDisplay(vecTime[3]); 
    std::vector<bool> num4leds = getSevenSegmentDisplay(vecTime[4]); 

    //Serial.print("NUM 1 LEDS: "); 
    //Serial.println(formatBoolVector(num1leds).c_str());
    //Serial.println(); 
    //Serial.print("NUM 2 LEDS: "); 
    //Serial.println(formatBoolVector(num2leds).c_str());
    //Serial.println(); 
    //Serial.print("NUM 3 LEDS: "); 
    //Serial.println(formatBoolVector(num3leds).c_str());
    //Serial.println(); 
    //Serial.print("NUM 4 LEDS: "); 
    //Serial.println(formatBoolVector(num4leds).c_str());    
    //Serial.println(); 

    changeNumLed(num1leds, "dig4", sec); //sec added for color
    changeNumLed(num2leds, "dig3", sec);
    changeNumLed(num3leds, "dig2", sec);
    changeNumLed(num4leds, "dig1", sec);

    //change colon to dash
    //if (vecTime[2]) {
    //  leds[48] =  getColor(sec); 
    //  leds[47] =  getColor(sec); 
    //  leds[46] =  CRGB::Black; 
    //}
    //else {
    //  leds[48] =  CRGB::Black; 
    //  leds[47] =  CRGB::Black; 
    //  leds[46] =  getColor(sec); 
    //}
    FastLED.show();
    //delay(39);
    //yield();

    
    return 1;
    

  //}
}

void lightBlueGear() {
  for (int i = 0; i < 4; i++) {
      leds[TEST_MAP["gear1"][i]] = CRGB::Blue; 
  }
  FastLED.show(); 
  delay(1);
}

void lightOrangeGear() {
  for (int i = 0; i < 4; i++) {
      leds[TEST_MAP["gear2"][i]] = CRGB::Orange; 
  }
  FastLED.show(); 
  delay(1);
}

unsigned char data[4];

void send(){
  data[0] = 0x1;
  data[1] = sendData.run;
  data[2] = sendData.done;
  data[3] = sendData.reset_ack;
  
  udp.beginPacket(IPAddress(224, 1, 1, 1), udpSend);
  udp.write(data, sizeof(data));
  udp.endPacket();
}


/*
function to write: 
- print from 2 minutes down
- set object to color
- ser number to 
*/
int state = 0; 
long timerVar = millis();
void loop() { 
  // Serial.println(1); 

  // testNumber(); 

  // testDigitInt(); 
  //Serial.println(runclock);
  //Serial.println(reset + " + " + runclock);
  // recieve data
  //if (connected) {
    int len = udp.parsePacket();
    if (len > 0) {
      Serial.print("src IP: ");
      Serial.print(udp.remoteIP());
      Serial.print(";    packet: [");
      char buf[len];
      udp.read(buf, len);
      for(int i = 0; i < len; i++){
        Serial.print(buf[i], HEX);
        Serial.print(", ");
      }
      Serial.println(" ");

      //unpack vals. thses might be backward.... 
      recData.startClock  = buf[0]?1:0;
      recData.reset       = buf[1]?1:0;
      recData.pause       = buf[2]?1:0;
      recData.win        = buf[3];
      
      recData.readyBlue   = buf[7]?1:0;
      recData.readyOrange = buf[8]?1:0;
      recData.orangeTapin = buf[9]?1:0;
      recData.blueTapin   = buf[10]?1:0;
      if(sendData.done && recData.startClock){
        recData.startClock = 0;
      }
      else if(sendData.done){
        sendData.done = 0; // should prevent an immediate restart. also means you cant just lean on the start button to get it to restart imediatly. I think thatts good 
      }
      if(recData.startClock && runclock == 0){
        runclock = 1;
        sendData.run = runclock;


        //sendData.done = 0; I dont think this is useful to me anymore.... 
      }
      

    }
    

  //}


  // I think ordering logic on this is is someone won -> do we reset -> is clock running (y -> clocking, n -> yellow)
  if(millis() - timerVar > 300){

  
    if( recData.readyBlue ){
      lightBlueGear();
      FastLED.show(); 
      
    }
    if( recData.readyOrange ){
      lightOrangeGear();
      FastLED.show(); 
  
    }
  
  
    if(recData.reset){
      runclock = 0;
      recData.reset = 0;
      setStartClock = 0;
      sendData.done = 1;
      //testIntegration(120.0, 0);

  
      // whatever other resting the clock stuffff.. (set black ? or ) 
      //send acknak 
      // Still need to set to black or something...
      Serial.println("THIS THING I THINK");
      sendData.reset_ack = 0;
      send();
      sendData.reset_ack = 0;
      setAlltoColor(CRGB::Black);

    }
    
    else if (recData.win != 0){
      Serial.println(recData.win);
      if(recData.win == 1){
        runclock = 0;
        sendData.run = runclock;
        setStartClock = 0;
        send();
        // orange win
        //whole clock winnigng color. 
        lightZeroWinnerColor("orange");
        Serial.println("ORANGEE WINN");
      }
      else if (recData.win == 2) {
        runclock = 0;
        sendData.run = runclock;
        setStartClock = 0;
        send();  
  
        //blue win
        lightZeroWinnerColor("blue");
        Serial.println("BLUEE WINNN AN N SNNNIN");
      }
      else{
        //wth ( as in thiis shpuldent happen)
      }
    }  
    else if (recData.startClock) {
      Serial.println("AHHHHHHHH RUNNNN");
      
      state = testIntegration(181.00, recData.pause);
      if ( state == 0 ){// indicating time ==0
        // yellow,  ( I think )
        goToBlack();
        // send done,
        sendData.done = 1;
        runclock = 0;
        sendData.run = runclock;
  
        send();
        delay(200);// just to make sure that the l;aptop gets the jist and I dont just start the clock agian immediatly. ill do somemore logic for this too
      } 
  
    }
    
  }

}