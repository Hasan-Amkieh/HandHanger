#include <Arduino.h>

#include <Servo.h> // The engines are to be changed into a stepper motor inside the aliexpress shop cart

#include "WiFi.h"
#include "ESPAsyncWebServer.h"

static const int anglePerSignalRate = 3;

static const int zAxisServoPin = 14; // increase for East, decrease for West
static const int  xAxisServoPin = 12; // increase for North, decrease for South

static const int northInPin = 5;
static const int eastInPin = 17;
static const int southInPin = 16;
static const int westInPin = 4;

Servo zAxisServo;
Servo xAxisServo;

int zAxisAngle = 90;
int xAxisAngle = 90;

const char* ssid = "Hand_Hanger_Project";
const char* password = "31415926"; // which is pi

AsyncWebServer server(80); // Core 0 is responsible for the server and the Wifi, the rest is left for Core 1

/*void createSemaphore() {
    commonVariablesLock = xSemaphoreCreateMutex();
    xSemaphoreGive( ( commonVariablesLock) );
}*/

SemaphoreHandle_t movementLock = xSemaphoreCreateMutex(); // for movement locks
SemaphoreHandle_t serialLock = xSemaphoreCreateMutex(); // for movement locks

// Lock the variable indefinietly. ( wait for it to be accessible )
void lockVariable(SemaphoreHandle_t* lock_) {
    xSemaphoreTake(*lock_, portMAX_DELAY);
}

void unlockVariable(SemaphoreHandle_t* lock_) {
    xSemaphoreGive(*lock_);
}

TaskHandle_t core1Handle = NULL;

void loop_1(void* arg);

// To format strings before passing them into println, use this function:

/*template<typename ... Args> // Requires C++11 :
std::string string_format(const std::__cxx11::string& format, Args ... args) {
    int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}*/

template<typename ... Args>
void print(Args ... args) {

  lockVariable(&serialLock);
  Serial.printf(args ...);
  Serial.flush();
  unlockVariable(&serialLock);

}

void setup() {

  xSemaphoreGive(movementLock);
  xSemaphoreGive(serialLock);

  Serial.begin(115200);
  if (!setCpuFrequencyMhz(240)) {
    Serial.printf("[WARNING][CORE_0] CPU speed has not changed into 240 MHz\n");
  }
  Serial.printf("[CORE_0] CPU Frequency is %d\n", getCpuFrequencyMhz());
  pinMode(northInPin, INPUT);
  pinMode(eastInPin, INPUT);
  pinMode(southInPin, INPUT);
  pinMode(westInPin, INPUT);
  if (!zAxisServo.attach(zAxisServoPin)) {
    Serial.printf("[ERROR][CORE_0] Z-Axis-Servo has not been attached!");
  }
  if (!xAxisServo.attach(xAxisServoPin)) {
    Serial.printf("[ERROR][CORE_0] X-Axis-Servo has not been attached!");
  }
  zAxisServo.write(zAxisAngle);
  xAxisServo.write(xAxisAngle);

  if (!WiFi.softAP(ssid, password)) {
    Serial.println("[ERROR][CORE_0] The network could not be created");
  } else {
    Serial.printf("[CORE_0] Network IP: %s\n", WiFi.softAPIP().toString());
  }

  // Assign Core 1:

  xTaskCreatePinnedToCore(loop_1, "Core1_Process", 4096, NULL, 10, &core1Handle, 1); // The fifth argument is the priority, the higher the more important

  server.on("/moveX", HTTP_GET, [] (AsyncWebServerRequest *request) {
    int angle = std::atoi(request->arg("angle").c_str());
    lockVariable(&movementLock);
    xAxisAngle += angle;
    unlockVariable(&movementLock);
    print("x angle became: %d\n", xAxisAngle);

    request->send_P(200, "text/plain", "moved");
  });

  server.on("/moveZ", HTTP_GET, [] (AsyncWebServerRequest *request) {
    int angle = std::atoi(request->arg("angle").c_str());
    lockVariable(&movementLock);
    zAxisAngle += angle;
    unlockVariable(&movementLock);

    print("z angle became: %d\n", zAxisAngle);
    
    request->send_P(200, "text/plain", "moved");
  });

  server.begin();

}

void loop() { // Core 0

  delay(1000);

}

void loop_1(void* args) { // Core 1

  print("[CORE_1] Core 1 has started running\n");

  if (!setCpuFrequencyMhz(240)) {
    print("[WARNING][CORE_1] CPU speed has not changed into 240 MHz\n");
  }
  print("[CORE_1] CPU Frequency is %d\n", getCpuFrequencyMhz());

  while(true) {

    if (digitalRead(northInPin)) {
      xAxisAngle -= anglePerSignalRate;
      Serial.printf("X axis angle %d\n", xAxisAngle);
      if (xAxisAngle < 0) {
        xAxisAngle = 0;
      }
    }
    if (digitalRead(southInPin)) {
      xAxisAngle += anglePerSignalRate;
      Serial.printf("X axis angle %d\n", xAxisAngle);
    
      if (xAxisAngle > 180) {
        xAxisAngle = 180;
      }
    }
    if (digitalRead(eastInPin)) {
      zAxisAngle -= anglePerSignalRate;
      ("Z axis angle %d\n", zAxisAngle);
      if (zAxisAngle < 0) {
        zAxisAngle = 0;
      }
    }
    if (digitalRead(westInPin)) {
      zAxisAngle += anglePerSignalRate;
      Serial.printf("Z axis angle %d\n", zAxisAngle);
      if (zAxisAngle > 180) {
        zAxisAngle = 180;
      }
    }
    lockVariable(&movementLock);
    zAxisServo.write(zAxisAngle);
    xAxisServo.write(xAxisAngle);
    unlockVariable(&movementLock);

    delay(5);

  }

}