

#include <analogWrite.h>
#include <Wire.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <arduino.h>



//wifi
WiFiUDP udp;
const char * ssid = "Taco";
const char * password = "tacotaco";
boolean connected = false;








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
      udp.beginMulticast(IPAddress(224, 1, 1, 1), 5007); //libmapper admin bus multicast
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


void add_int_to_raw(unsigned char* raw, int index, int v){
  raw[index] = v&0x00FF;
  raw[index+1] = (v&0x00FF00)>>8;
  raw[index+2] = (v&0x00FF0000)>>16;
  raw[index+3] = (v&0xFF000000)>>24;
}



void setup()
{
  
  Serial.begin(115200);
  connectToWiFi(ssid, password);
  Wire.begin(); 
  


}
int data[5];




void loop()
{
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
      x = buf[0]; 
      x += buf[1] << 8;
      x += buf[2] << 16;
      x += buf[3] << 24;
      y = buf[4];
      y += buf[5] << 8;
      y += buf[6] << 16;
      y += buf[7] << 24;
      up = buf[8]?1:0;
      down = buf[9]?1:0;
      left = buf[10]?1:0;
      right = buf[11]?1:0;
    
    }
    

  }
  

  if(connected){
    unsigned char raw[24];
    data[0] = USDist;
    data[1] = temp;
    data[2] = humid;
    data[3] = co2;
    data[4] = tvoc;
    data[5] = hall;
    for(int i = 0; i<6; i++){
      add_int_to_raw(raw, i*4, data[i]);
    }
    udp.beginPacket("224.1.1.1", 5006);
    udp.write(raw, sizeof(int)*6);
    udp.endPacket();
  }
  
}
