// server code

#include <Adafruit_DotStar.h>
#include <WiFi.h>
#include <esp_now.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

#define TAPOUT_PIN 10
#define READY_PIN 11
#define NUMPIXELS 30  // Number of LEDs in strip
#define DATA_PIN 12
#define CLOCK_PIN 13
Adafruit_DotStar strip(NUMPIXELS, DATA_PIN, CLOCK_PIN, DOTSTAR_RGB);

const char *ssid = "RobobrawlButton";
const char *password = "doyourobotics";

//IPAddress local_ip(192, 168, 0, 1);
//IPAddress gateway(192, 168, 0, 1);
//IPAddress subnet(255, 255, 255, 0);
//
//WebServer server(80);


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

//void sendESPNOW(int ready, int tapout, Mode mode) {
//  outgoingValues.ready = ready;
//  outgoingValues.tapout = tapout;
//  outgoingValues.mode = mode;
//  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&outgoingValues, sizeof(outgoingValues));
//
//  if (result != ESP_OK) {
//    Serial.println("Error sending data (button)");
//  }
//}
//
//void sendESPNOWclock() {
//  
//
//  esp_err_t result = esp_now_send(clockAddress, (uint8_t *)&clock_outgoing, sizeof(clock_outgoing));
//
//  if (result != ESP_OK) {
//    Serial.println("Error sending data (clock)");
//  }
//
//}
//
//
//// Callback when data is sent
//void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
//  Serial.print("\r\nLast Packet Send Status: ");
//  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
//}
//
//// callback when data is recv from Master
//void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int data_len) {
//  char macStr[18];
//  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
//           info->src_addr[0], info->src_addr[1], info->src_addr[2], info->src_addr[3], info->src_addr[4], info->src_addr[5]);
//  memcpy(&incomingValues, data, sizeof(incomingValues));
//  Serial.print("Last Packet Recv from: ");
//  Serial.println(macStr);
//  Serial.print("Last Packet Recv Data: ");
//  Serial.print(incomingValues.ready);
//  Serial.println(incomingValues.tapout);
//  Serial.println("");
//  if (!blueReady && incomingValues.ready) {
//    blueReady = 1;
//    sendESPNOW(0, 0, Mode::green);
//  }
//  if (!blueTapout && !orangeTapout && incomingValues.tapout) {
//    blueTapout = 1;
//    clock_outgoing.blueTapout = 1;
//    sendESPNOW(0, 0, Mode::red);
//    setStrip(Mode::yellow);
//  }
//  sendESPNOWclock();
//
//
//  server.send(200, "text/html", SendHTML());
//}



void connectToWiFi(const char * ssid, const char * pwd) {
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
  switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      //When connected set
      Serial.print("WiFi connected! IP address: ");
      Serial.println(WiFi.localIP());
      udp.beginMulticast(IPAddress(224, 1, 1, 1), 5008); //libmapper admin bus multicast ----------- look at the port for this, make sure its right.
      //udp.begin(7000); //test regular UDP server endpoint
      connected = true;
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      WiFi.begin(ssid, password);
      connected = false;
      break;
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
  connectToWiFi(ssid, password);


  pinMode(TAPOUT_PIN, INPUT_PULLDOWN);
  pinMode(READY_PIN, INPUT_PULLDOWN);

  strip.begin();
  strip.setBrightness(150);
  strip.show();


  clock_outgoing.bothReady = 0;
  clock_outgoing.blueTapout = 0;
  clock_outgoing.orangeTapout = 0;
  clock_outgoing.reset = 0;




  // Register for a callback function that will be called when data is received

  delay(100);
  // I dont think any of this will matter, but im scared
  server.on("/", handle_OnConnect);
  server.on("/identifyteams", handle_IdentifyTeams);
  server.on("/resetmatch", handle_ResetMatch);
  server.on("/orangewin", handle_OrangeWin);
  server.on("/bluewin", handle_BlueWin);
  server.on("/getOrange", handle_OrangeStatus);
  server.on("/getBlue", handle_BlueStatus);
  server.on("/startclock", handle_startClock);
  server.onNotFound(handle_NotFound);
//
  server.begin();
  Serial.println("HTTP server started");
}

float last_mill = 0;

long timerVal = millis()

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


  

  if (connected) {
    int len = udp.parsePacket();
    if (len > 0) {
      Serial.print("src IP: ");
      Serial.print(udp.remoteIP());
      Serial.print(";    packet: [");
      char buf[len];
      udp.read(buf, len);
      //for(int i = 0; i < len; i++){
      //  Serial.print(buf[i], HEX);
       // Serial.print(", ");
      //}
      //Serial.println(" ");
      rec_data.tap_ack  = buf[1]?1:0;
      rec_data.main_ack = buf[0]?1:0;
      rec_data.color  = buf[2];
      rec_data.color += buf[3] << 8;
      rec_data.color += buf[4] << 16;
      rec_data.color += buf[5] << 24;

      if( rec_data.main_ack && send_data.main_press){
        send_data.main_press = 0;
      }
      if( rec_data.tap_ack && send_data.tap_press){
        send_data.tap_press = 0;
      }


      // NOTE::: 0-4 will be the from ID when I send thesesese. 
    }
    

  }


  switch (rec_data.color)
  {
  case 0:
    setStrip(yellow);
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
    send();
  }

}
void handle_OnConnect() {
  server.send(200, "text/html", SendHTML());
}
void handle_IdentifyTeams() {
  setStrip(Mode::orange);
  outgoingValues.ready = 0;
  outgoingValues.tapout = 0;
  outgoingValues.mode = Mode::blue;
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&outgoingValues, sizeof(outgoingValues));

  if (result != ESP_OK) {
    Serial.println("Error sending data");
  }

  server.send(200, "text/html", SendHTML());
}
void handle_ResetMatch() {
  orangeReady = 0;
  orangeTapout = 0;
  blueReady = 0;
  blueTapout = 0;

  setStrip(Mode::orange);

  clock_outgoing.bothReady = orangeReady && blueReady;
  clock_outgoing.blueTapout = blueTapout;
  clock_outgoing.orangeTapout = orangeTapout;
  clock_outgoing.reset = 1;
  sendESPNOWclock(); 
  sendESPNOW(0, 0, Mode::blue);

  server.send(200, "text/html", SendHTML());
  delay(200);
  clock_outgoing.reset = 0;


}
void handle_OrangeWin() {
  setStrip(Mode::green);
  sendESPNOW(0, 0, Mode::red);
  clock_outgoing.blueTapout = 1;
  blueTapout = 1;
  sendESPNOWclock();
  server.send(200, "text/html", SendHTML());
}
void handle_BlueWin() {
  setStrip(Mode::red);
  sendESPNOW(0, 0, Mode::green);

  orangeTapout = 1;
  clock_outgoing.orangeTapout = 1;
  sendESPNOWclock();

  server.send(200, "text/html", SendHTML());
}

void handle_OrangeStatus() {
  String returnVal = "NOT READY";
  if (orangeReady) {
    returnVal = "READY";
  }
  if (orangeTapout) {
    returnVal = "TAPOUT";
  }
  server.send(200, "text/plane", returnVal);
}
void handle_BlueStatus() {
  String returnVal = "NOT READY";
  if (blueReady) {
    returnVal = "READY";
  }
  if (blueTapout) {
    returnVal = "TAPOUT";
  }
  server.send(200, "text/plane", returnVal);
}
void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

void handle_startClock() {
  // IDK waht to do to start it, or if I shjould do a ready check or ewhattttt, 
  if(orangeReady && blueReady){
    clock_outgoing.bothReady = 1;
    sendESPNOWclock();

  }
  else{
    Serial.print("start clock pressed but both not ready");
  }

  server.send(200, "text/html", SendHTML());

}

String SendHTML() {
  String ptr = R"***(<!DOCTYPE html> <html>
  <head><meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>Robobrawl Ready Button Interface</title>
  <style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
  body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}
  .button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}
  .button-on {background-color: #3498db;}
  .button-on:active {background-color: #2980b9;}
  .button-off {background-color: #34495e;}
  .button-off:active {background-color: #2c3e50;}
  p {font-size: 30px;color: #888;margin-bottom: 10px;}
  </style>
  </head>
  <body>
  <h1>Robobrawl Ready Button Interface</h1>
  <a class="button button-off" href="/identifyteams">Identify Teams</a>
  <a class="button button-off" href="/resetmatch">Reset Match</a>
  <a class="button button-off" href="/orangewin">Orange Win</a>
  <a class="button button-off" href="/bluewin">Blue Win</a>
  <a class="button button-off" href="/startclock">Start Clock    (only happens if both are ready.)</a>

    <h2>
      Orange: <span id="orange">NULL</span>
      Blue: <span id="blue">NULL</span>
    </h2>
  

  <script> setInterval(
    function() {
    getOrangeStatus();
    getBlueStatus();
  }, 500);
  function getOrangeStatus() {
    var statusRequestOrange = new XMLHttpRequest();
    statusRequestOrange.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("orange").innerHTML =
          this.responseText;
      }
    };
    statusRequestOrange.open("GET", "getOrange", true);
    statusRequestOrange.send();
  }
  function getBlueStatus() {
    var statusRequestBlue = new XMLHttpRequest();
    statusRequestBlue.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("blue").innerHTML =
          this.responseText;
      }
    };
    statusRequestBlue.open("GET", "getBlue", true);
    statusRequestBlue.send();
  }
  </script>;
  </body>;
  </html>;)***";
  return ptr;
}