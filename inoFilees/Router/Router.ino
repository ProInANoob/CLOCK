
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

const char *ssid = "robobrawlbutton";
const char *password = "doyourobotics";

const char * udpAddress = "224.1.1.1";
const int udpClock = 5007;
const int udpOrange = 5008;
const int udpBlue = 5009;
const int udpRecieve = 5006;

WiFiUDP udp;

struct fromButtonData{
  bool mainPress = 0;
  bool tapPress = 0;
};

struct toButtonData{
  int color = 0;
  bool main_ack = 0;
  bool tap_ack = 0;
};

struct fromClockData{
  bool running = 0;
  bool reset_ack = 0;
  bool done = 0;

};

struct toClockData{
  bool startClock = 0;
  bool reset = 0;
  bool pause = 0;
  int win = 0;
  bool readyBlue = 0;
  bool readyOrange = 0;
  bool tapinBlue = 0;
  bool tapinOrange = 0;
};


struct FromComp{
  bool reset = 0;
  bool pause = 0;
  bool orangeTap = 0;
  bool blueTap = 0;
  bool start = 0;
  bool Owin = 0;
  bool Bwin = 0;
};
FromComp fromComp;

fromButtonData fromBlue;
fromButtonData fromOrange;
toButtonData toBlue;
toButtonData toOrange;
fromClockData fromClock;
toClockData toClock;
unsigned char Bdata[3];
void sendBlue(){
  Bdata[0] = toBlue.color;
  Bdata[1] = toBlue.main_ack; // not suere of the lauoyt here...
  Bdata[2] = toBlue.tap_ack;

  udp.beginPacket(IPAddress(224,1,1,1), udpBlue );
  udp.write(Bdata, sizeof(char)*3);
  udp.endPacket();


}

unsigned char Odata[3];
void sendOrange(){
  Odata[0] = toOrange.color;
  Odata[1] = toOrange.main_ack; // not suere of the lauoyt here...
  Odata[2] = toOrange.tap_ack;

  udp.beginPacket(IPAddress(224,1,1,1), udpOrange );
  udp.write(Odata, sizeof(char)*3);
  udp.endPacket();

}

unsigned char Cdata[8];
void sendClock(){
  Cdata[0] = toClock.startClock;
  Cdata[1] = toClock.reset;
  Cdata[2] = toClock.pause;
  Cdata[3] = toClock.win;
  Cdata[4] = toClock.readyBlue;
  Cdata[5] = toClock.readyOrange;
  Cdata[6] = toClock.tapinBlue;
  Cdata[7] = toClock.tapinOrange;

  udp.beginPacket(IPAddress(224,1,1,1), udpClock );
  udp.write(Cdata, sizeof(char)*8);
  udp.endPacket();


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
      //udp.beginMulticast(IPAddress(244, 1, 1, 1), 5008);
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

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));
}

TaskHandle_t Task1;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // delete old config
  WiFi.disconnect(true);

  delay(1000);
  pinMode(2, OUTPUT);
  // Examples of different ways to register wifi events;
  // these handlers will be called from another thread.

  // Remove WiFi event
  //Serial.print("WiFi Event ID: ");
  //Serial.println(eventID);
  // WiFi.removeEvent(eventID);

  WiFi.begin(ssid, password);

  Serial.println();
  Serial.println();
  Serial.println("Wait for WiFi... ");

  delay(500);

  udp.beginMulticast(IPAddress(224, 1, 1, 1), 5006);
  xTaskCreatePinnedToCore(
      Task1code, /* Function to implement the task */
      "Task1", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      0,  /* Priority of the task */
      &Task1,  /* Task handle. */
      0); /* Core where the task should run */
}

void Task1code( void * parameter) {
  for(;;) {
    if (Serial.available() > 0) {

    String command = Serial.readStringUntil('\n');
    command.trim();
    //do whatever. 
    char* ptr;

    long res = strtol(command.c_str(), &ptr, 16 );
    Serial.println(res>>3);

    fromComp.start = (res>>2)%2;
    fromComp.blueTap = (res>>3)%2;
    fromComp.orangeTap = (res>>4)%2;
    fromComp.pause = (res>>5)%2;
    fromComp.reset = (res>>6)%2;

    if(fromComp.start){
      toClock.startClock = 1;
      fromComp.start = 0;
    }
    if(fromComp.blueTap){
      toClock.tapinBlue = 1;
      fromComp.blueTap = 0;
    }
    if(fromComp.orangeTap){
      toClock.tapinOrange = 1;
      fromComp.orangeTap = 0;
    } 
    if(fromComp.pause){
      toClock.pause = 1;
      fromComp.pause = 0;
    }
    if(fromComp.reset){
      toClock.reset = 1;
      toClock.pause = 0;
      toClock.tapinOrange = 0;
      toClock.tapinBlue = 0;
      toClock.win = 0;
      toClock.readyOrange = 0;
      toClock.readyBlue = 0;
      toClock.startClock = 0;
      fromComp.reset = 0;
      sendClock();
      toClock.reset = 0; 
    }
      
  }
}
}

int timerVal = millis();

void loop() {
  // put your main code here, to run repeatedly:

  digitalWrite(2,toClock.startClock);
  
  int len = udp.parsePacket();
  //Serial.println(len);
  if (len > 0) {
  //Serial.print("src IP: ");
  //Serial.print(udp.remoteIP());
  //Serial.print(";    packet: [");
  char buf[len];
  udp.read(buf, len);
  for(int i = 0; i < len; i++){
    Serial.print(buf[i], HEX);
    Serial.print(", ");
  }
  int ID = buf[0];
  int v1 = buf[1];
  int v2 = buf[2];
  int v3 = 0;
  if(ID == 1){
    v3 = buf[3];
  }
  int val ;
  switch (ID){
    case 1: // refdo theses.
      //from clock
      fromClock.running =       v1;
      fromClock.done =      v2;
      fromClock.reset_ack = v3;

      break;
    case 2:
      fromOrange.mainPress = v1;
      fromOrange.tapPress =  v2;

      
      //from Orange
      
      break;
    case 3:
      //from blue

      fromBlue.mainPress = v1;
      fromBlue.tapPress =  v2;



      break;
    default:
    break;
      
  }
  }

  
    //well with this I gotta send the things I get back to the pc, but I can do that. 
    // ohh shiii it uses print to get data back. oh well., thatll be interssetingg 

    if(millis() - timerVal > 300 ){

      sendClock();
      sendOrange();
      sendBlue();
        
    }

    Serial.println(fromOrange.mainPress);

    if(fromBlue.tapPress == 1 || fromOrange.tapPress == 1){
      if(fromBlue.tapPress == 1){
        toClock.win = 1;
      }
      else if(fromOrange.tapPress == 1){
        toClock.win = 2;
      }
    
      if(toClock.win == 1){
        toBlue.tap_ack = 1;
      }
      else{
        toBlue.tap_ack = 1;
      }

    }

    if(toClock.reset == 1 && fromClock.reset_ack == 1){
      toClock.reset = 0;
    }

    if(fromBlue.mainPress == 1 && toClock.tapinBlue == 1){
      toClock.readyBlue = 1; 
      toBlue.main_ack = 1;
    }
    else if ( fromBlue.mainPress == 1){
      toBlue.main_ack = 1;
    }
    else{
      toBlue.main_ack = 0;
    }

    if(fromOrange.mainPress == 1 && toClock.tapinOrange == 1){
      toClock.readyOrange = 1;
      toOrange.main_ack = 1;
    }
    else if(fromOrange.mainPress == 1){
      toOrange.main_ack = 1;

    }
    else{
      toOrange.main_ack = 0;
    }

    if(fromClock.done == 1){
      toClock.startClock = 0;
    }

    toBlue.color = 0;
    toOrange.color = 0;

    if(toClock.startClock == 1){
      toBlue.color = 1;
      toOrange.color = 1;
    }
    if(toClock.tapinOrange == 1){
      //Serial.println("TAP BLUEE");
      if(toClock.readyOrange == 1){
        toBlue.color = 1;
        Serial.println("AHAHHHHHHHHHH");
      }
      else{
        toOrange.color = 2;
      }
    }

    if(toClock.tapinBlue == 1){
      //Serial.println("TAP BLUEE");
      if(toClock.readyBlue == 1){
        toOrange.color = 1;
        Serial.println("__________________");
      }
      else{
        toBlue.color = 2;
      }
    }



    if(toClock.win == 1){
      toOrange.color = 3;
      toBlue.color = 2;
    }
    else if(toClock.win == 2){
      toOrange.color = 2;
      toBlue.color = 3;
    }

    //Serial.println(res);
    //if(res%2 == 1){
    //  toClock.startClock = 1;
    //}
    //if((res>>1)%2 == 1){
    //  toClock.reset = 1;
    //}
    //if((res>>2)%2 == 1){
    //  toClock.pause = 1;
    //}
    //if((res>>3)%2 == 1){
    //  toClock.win = 1; // ??? mabey? this is Blue lost, Orange won.
    //}
    //if((res>>4)%2 == 1){
    //  toClock.win = 2;
    //}
    //if((res>>5)%2 == 1){
    //  toClock.tapinBlue = 1;
    //}
    //if((res>>6)%2 == 1){
    //  toClock.tapinOrange = 1;
    //}
  }
  //need logic here too for the trasintion states. 
  // or I could do teh logic on the python and send over the finsifhed struct instead. then just send those. 
  //Serial.println(toBlue.color);
  //if(toBlue.color != 0){
  //  digitalWrite(2, 1);
  //}
  //else{
  //  digitalWrite(2, 0);
  //}




