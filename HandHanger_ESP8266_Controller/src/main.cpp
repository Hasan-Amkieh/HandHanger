#include <Arduino.h>

// This is the client side, it sends commands to the server-side (IR receiver)

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiClient; // use (WiFiMulti.run() == WL_CONNECTED) to check the state

const char* moveXServerCommand = "http://192.168.4.1/moveX?angle=";
const char* moveZServerCommand = "http://192.168.4.1/moveZ?angle=";

const char* ssid = "Hand_Hanger_Project";
const char* password = "31415926"; // which is pi

String httpGETRequest(const char* url) {
  WiFiClient client;
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(client, url);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "--"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1015 ads;

void setup() {

  Serial.begin(9600);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to WiFi");

  ads.setGain(GAIN_ONE); // 1 bit = 2mv
  if (!ads.begin()) {
    Serial.println("The ADS could not be ");
  }

  Serial.println("Everything is set!");

  //pinMode(D2, OUTPUT);
}

float movementSpeed = 1.0f, moveX = 0, moveY = 0;
int toMoveX = 0, toMoveZ = 0;

void loop() {

  movementSpeed = ads.computeVolts(ads.readADC_SingleEnded(0));
  moveX = ads.computeVolts(ads.readADC_SingleEnded(1));
  moveY = ads.computeVolts(ads.readADC_SingleEnded(2));

  Serial.printf("%f - %f - %f\n", movementSpeed, moveX, moveY);

  toMoveX = (int)((moveX - 1.63) / 0.5); // each half will be a a single movement unit
  toMoveX = toMoveX * movementSpeed;

  toMoveZ = (int)((moveY - 1.63) / 0.5); // each half will be a a single movement unit
  toMoveZ = toMoveZ * movementSpeed;

  String str = String(moveXServerCommand);
  str.concat(String(toMoveX));
  httpGETRequest(str.c_str());

  str = String(moveZServerCommand);
  str.concat(String(toMoveZ));
  httpGETRequest(str.c_str());

  delay(800);

  /*digitalWrite(D2, HIGH);
  delay(2000);
  digitalWrite(D2, LOW);
  delay(1000);*/
}