// server code (ORANGE button).. probably.

#include <Adafruit_DotStar.h>
#include <WiFi.h>
#include <esp_now.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>


#define TAPOUT_PIN 14 // go oback to 10
#define READY_PIN 15 // abck to 11
#define NUMPIXELS 30  // Number of LEDs in strip
#define DATA_PIN 12
#define CLOCK_PIN 13
Adafruit_DotStar strip(NUMPIXELS, DATA_PIN, CLOCK_PIN, DOTSTAR_RGB);

const char *ssid = "tacotaco";
const char *password = "tacotaco";

IPAddress local_ip(192, 168, 0, 1);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
//
WiFiServer server(80);


//wifi
WiFiUDP udp;
boolean connected = false;


int readyState;             // the current reading from the input pin
int lastReadyState = LOW;   // the previous reading from the input pin
int tapoutState;            // the current reading from the input pin
int lastTapoutState = LOW;  // the previous reading from the input pin

unsigned long lastReadyDebounceTime = 0;   // the last time the output pin was toggled
unsigned long lastTapoutDebounceTime = 0;  // the last time the output pin was toggled

unsigned long debounceDelay = 50;  // the debounce time; increase if the output flickers




enum Mode { green,
            yellow,
            red,
            orange,
            blue,
            none };




int orangeReady = 0;
int orangeTapout = 0;
int blueReady = 0;
int blueTapout = 0;

void setStrip(Mode color) {
  for (int i = 0; i < NUMPIXELS; i++) {
    switch (color) {
      case green:
        strip.setPixelColor(i, 0x0000FF);
        break;
      case yellow:
        strip.setPixelColor(i, 0x880088);
        break;
      case red:
        strip.setPixelColor(i, 0xFF0000);
        break;
      case orange:
        strip.setPixelColor(i, 0xFF0088);
        break;
      case blue:
        strip.setPixelColor(i, 0x00FF00);
        break;
      case none:
        strip.setPixelColor(i, 0x000000);
        break;
    }
  }
  strip.show();
}

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
      udp.beginMulticast("224.1.1.1", 5009);
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

struct  recieveData
{
  int color;
  bool main_ack;
  bool tap_ack;
};

recieveData rec_data;


struct sendData
{
  bool main_press;
  bool tap_press;
  /* data */
};

sendData send_data;

unsigned char data[6];

void send(){
  data[0] = 0x2;
  data[1] = 0x0;
  data[2] = 0x0;
  data[3] = 0x0;
  data[4] = send_data.main_press;
  data[5] = send_data.tap_press;
  
  udp.beginPacket("224.1.1.1", 5006);
  udp.write(data, sizeof(int)*6);
  udp.endPacket();
}



void setup() {
  Serial.begin(115200);
  Serial.println("THETOP");
  //WiFi.mode(WIFI_AP);
  delay(100);
  //Serial.println("JJJJSJSSJS");
  WiFi.onEvent(WiFiEvent);
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  //Serial.println("ASDASDASD");
  server.begin();
  Serial.println("HTTP server started");

  udp.beginMulticast(IPAddress(224, 1, 1, 1), 5008); 
  Serial.println("udp begin");


  pinMode(TAPOUT_PIN, INPUT_PULLDOWN);
  pinMode(READY_PIN, INPUT_PULLDOWN);

  strip.begin();
  strip.setBrightness(150);
  strip.show();






}

float last_mill = 0;

long timerVal = millis();

void loop() {

  int readyReading = digitalRead(READY_PIN);
  if (readyReading != lastReadyState) {
    // reset the debouncing timer
    lastReadyDebounceTime = millis();
  }
  if ((millis() - lastReadyDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (readyReading != readyState) {
      readyState = readyReading;

      // only toggle the LED if the new button state is HIGH
      if ((readyState == HIGH) ) {
        send_data.main_press = 1;
      }
    }
  }

  int tapoutReading = digitalRead(TAPOUT_PIN);
  if (tapoutReading != lastTapoutState) {
    // reset the debouncing timer
    lastTapoutDebounceTime = millis();
  }
  if ((millis() - lastTapoutDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (tapoutReading != tapoutState) {
      tapoutState = tapoutReading;
        if(tapoutState == 1){
          send_data.tap_press = 1;
        }

      }
  }


  

  
  int len = udp.parsePacket();
  if (len > 0) {
    Serial.print("src IP: ");
    Serial.print(udp.remoteIP());
    Serial.print(";    packet: [");
    char buf[len];
    udp.read(buf, len);
    //for(int i = 0; i < len; i++){
    //  Serial.print(buf[i], HEX);
    //  Serial.print(", ");
    //}
    Serial.println(" ");
    rec_data.tap_ack  = buf[1]?1:0;
    rec_data.main_ack = buf[0]?1:0;
    rec_data.color  = buf[2];
    rec_data.color += buf[3] << 8;
    rec_data.color += buf[4] << 16;
    rec_data.color += buf[5] << 24;
    Serial.print(rec_data.color, BIN);
    if( rec_data.main_ack && send_data.main_press){
      send_data.main_press = 0;
    }
    if( rec_data.tap_ack && send_data.tap_press){
      send_data.tap_press = 0;
    }


      // NOTE::: 0-4 will be the from ID when I send thesesese. 
    }
    

  


  switch (rec_data.color)
  {
  case 0:
    setStrip(yellow); // imcalling this the standby color... 
    break;
  case 1:
    setStrip(green);
    break;
  case 2:
    setStrip(orange);
    break;
  case 3:
    setStrip(red);
  default:
    break;
  }


  // Serial.print("Ready(Button: ");
  // Serial.print(readyReading);
  // Serial.print(", State: ");
  // Serial.print(lastReadyState);
  // Serial.print(") Tapout(Button: ");
  // Serial.print(tapoutReading);
  // Serial.print(", State: ");
  // Serial.print(lastTapoutState);
  // Serial.print(")");
  // Serial.println();
  lastReadyState = readyReading;
  lastTapoutState = tapoutReading;
  if(millis() - timerVal > 100){
    //send();
  }

}




