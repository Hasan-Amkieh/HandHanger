#include <Arduino.h>

// This is the client side, it sends commands to the server-side (IR receiver)

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiClient; // use (WiFiMulti.run() == WL_CONNECTED) to check the state

const char* moveXServerCommand = "http://192.168.4.1/moveX?angle=";
const char* moveZServerCommand = "http://192.168.4.1/moveZ?angle=";


#include <IRremoteESP8266.h>
#include <IRsend.h>

#include <DigiPotX9Cxxx.h>


const uint16_t kIrLed = D0; // which is GPIO 16
//IRsend irsend(kIrLed);

DigiPot pot(2, 0, 15); // D4, D3, D8 accordingly

static const uint32_t moveCommand = 100;

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

void setup() {

  Serial.begin(9600);
  //irsend.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  Serial.println("Everything is set!");

  pot.set(70); // MIN is "27" and MAX is "99"

  pinMode(D0, OUTPUT);
}

void loop() {


  /*uint32_t data = moveCommand;
  irsend.sendNEC(data, 32);
  delay(2000);*/

  pot.increase(10);
  digitalWrite(D0, HIGH);
  delay(2000);
  digitalWrite(D0, LOW);
  delay(1000);

  Serial.printf("Steps: %d\n", pot.get());
  if (pot.get() >= 99) {
    Serial.println("Resetting to 0");
    pot.reset();
  }


}