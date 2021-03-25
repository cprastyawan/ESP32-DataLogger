#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>

String SERVER_NAME = "http://mywebwafiq16.000webhostapp.com/Lentera_Bumi_Nusantara/simpan.php?";
#define mqtt_port 12245
#define MQTT_USER "wafiq"
#define MQTT_PASSWORD "12345678"

const char *WIFI_SSID = "SETO";
const char *WIFI_PASS = "nurcahyo";


const char* mqtt_server = "l40840f6.en.emqx.cloud"; //"m16.cloudmqtt.com";


HTTPClient http;
bool start = false;

typedef union{
  uint8_t c8[4];
  float f32;
} Floating;

Floating pressure, temperature, current, voltage;

const uint8_t CSPin = 10;
uint32_t previousMillis = 0;


void Serial2String();
uint8_t checkCommand(String str);
void UpdateToServer();

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 16, 17);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connect.");
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected!");
  /*while(!start){
    if(Serial2.available() > 0){

    }
  }*/
  previousMillis = millis();
  Serial.println("Mulai!");
}

void loop(){
  if(Serial2.available() > 0){
    Serial2String();
  }

  if(millis() - previousMillis >= 5500){
    previousMillis = millis();

    UpdateToServer();
  }
}

void Serial2String(){
  static bool captureData;
  char rxChar;
  static String rxString;
  rxChar = Serial2.read();
  //Serial.print(rxChar);

  if(rxChar == '#' && !captureData){
    captureData = true;
  } else if(rxChar == 'S' && captureData){
    captureData = false;
    checkCommand(rxString);
    rxString.clear();
  } else if(captureData){
    rxString += rxChar;
  }
}

uint8_t checkCommand(String str){
  Floating temp;

  for(int i = 0; i < 4; i++){
    temp.c8[i] = str.charAt(i + 1); 
  }

  if(str.compareTo("begin") == 0){
    start = true;
    return 1;
  } else if(str.charAt(0) == 'T'){
    temperature.f32 = temp.f32;
    Serial.println(String("Temperature: ") + String(temperature.f32));
    return 2;
  } else if(str.charAt(0) == 'P'){
    pressure.f32 = temp.f32;
    Serial.println(String("Pressure: ") + String(pressure.f32));
    return 3;
  } else if(str.charAt(0) == 'I'){
    current.f32 = temp.f32;
    Serial.println(String("Current: ") + String(current.f32));
    return 4;
  } else if(str.charAt(0) == 'V'){
    voltage.f32 = temp.f32;
    Serial.println(String("Voltage: ") + String(voltage.f32));
    return 5;
  }

  return 0;
}

void UpdateToServer(){
  HTTPClient http;
  char content[200];
  
  String serverPath = SERVER_NAME + String("arah_angin=") + String(0) + String("&kecepatan_angin=") + String(0) + String("&suhu_udara=") + String(temperature.f32) + String("&tekanan_udara=") + String(pressure.f32) + String("&tegangan=") + String(voltage.f32) + String("&arus=") + String(current.f32);
  Serial.println(serverPath);

  http.begin(serverPath);

  int httpResponseCode = http.GET();

  if(httpResponseCode > 0){
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}