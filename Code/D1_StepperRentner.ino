/*
   Rentner-Uhr.
   siehe https://github.com/kfeger/Rentneruhr
   Sinn ist die Anzeige des Wochentags auf einer
   großen Skala.
   Die Position wird alle 3 Stunden aktualisiert.
   Die jeweiligen 0:00-Positionen wurden wegen Ungenauigkeite per Hand
   ausgemessen und festgelegt.
   Die Uhrzeit kommt von NTP.
   Wochentag: tm_wday, Stunde: tm_hour
*/
#include <ESP8266WiFi.h>
#include <AccelStepper.h>
#include "time.h"
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h> //für http-Update


// Stepper
const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution

// ULN2003 Motor Driver Pins
#define IN1 D5
#define IN2 D6
#define IN3 D7
#define IN4 D8

#define MAX_POS 1710
#define MIN_POS 0
#define HALF_POS 1024
#define OFFSET_STEPPER 188  // vom Anschlag aus
#define STEPS_PER_DAY 230   //Schritte pro 24h
#define STEPS_PER_REVOLUTION 2048
#define STEPS_3H 30
#define STEPS_1H 9
//#define STEPPER_OFF
bool HomeRun = false;
int OldPosition, CurrentPosition;

// Definitionen edr 0:00h-Position
const int Day0Position[8] = {
  0,
  241,  // Di 0h
  478,  // Mi
  706,  // Do
  934,  // Fr
  1178, // Sa
  1425, // So
  1680
};
const int StepWidth[7] = {
  10,
  10,
  9,
  9,
  10,
  10,
  10
};
int DayIndex = 0;

const char* Wochentag[8] = {
  "Sonntag",
  "Montag",
  "Dienstag",
  "Mittwoch",
  "Donnerstag",
  "Freitag",
  "Samstag",
  "Fehler"
};

ADC_MODE(ADC_VCC);

bool UpDown = true;
int Index;
// initialize the stepper library
AccelStepper stepper(AccelStepper::FULL4WIRE, IN1, IN3, IN2, IN4);

// MQTT
WiFiClient espClient;

//NTP
#define MY_NTP_SERVER "pool.ntp.org"
#define MY_TZ "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00"
time_t now;
tm tm;
char TimeChar[80];

// WiFi settings
/**/
const char* ssid = "orange";
const char* password = "w!r messen 1000 Werte";
/**/
// Manni.42
/*
const char* ssid = "vielleicht";
const char* password = "#xmanni19.195501";
*/
// Dietmar
/*
const char* ssid = "Hacker";
const char* password = "2253210679";
*/
char Hostname[32];
long lastReconnectAttempt = 0;
int PayloadValue = 0;
bool NewData = false;

// Webserver
ESP8266WebServer server(80); //Server on port 80
ESP8266HTTPUpdateServer serverUpdater;


//Projekt
String BaseFile;


void setup() {
  // Dateinamen holen und parsen
  BaseFile = String(__BASE_FILE__);
  int LastSlash = BaseFile.lastIndexOf('\\');
  String BaseFileSub = BaseFile.substring(LastSlash + 1);
  BaseFile = BaseFileSub.substring(0, BaseFileSub.length() - 8);
  #ifdef STEPPER_OFF
    BaseFile += " (Stepper ausgeschaltet)";
    #pragma message ("Stepper ausgeschaltet")
  #else
    BaseFile += " (Stepper dauernd an)";
    #pragma message ("Stepper dauernd an")
  #endif
  // fertig
  // initialize the serial port
  Serial.begin(115200);
  delay(100);
  Serial.println(" ");
  Serial.println("WLAN-Suche anzeigen");
  stepper.setMaxSpeed(400);
  stepper.setAcceleration(100);
  HalfPos();
  randomSeed(ESP.getVcc());
  WiFi.mode(WIFI_STA);
  byte macAddr[6];
  WiFi.macAddress(macAddr);
  sprintf(Hostname, "Rentner-%02x-%02x-%02x", macAddr[3], macAddr[4], macAddr[5]);
  WiFi.hostname(Hostname);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  bool updown = false;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  WiFi.macAddress(macAddr);
  Serial.println("");
  Serial.println("WLAN verbunden");
  Serial.println(BaseFile);
  // Print the IP address
  Serial.print("IP-Adresse: ");
  Serial.println(WiFi.localIP());
  delay(100);
  configTzTime(MY_TZ, MY_NTP_SERVER);
  while (time(&now) < 1000) { //ungültige Zeit
    delay(1000);  //Schamfrist für NTP
    configTzTime(MY_TZ, MY_NTP_SERVER);
    Serial.println("Warte auf NTP");
  }
  localtime_r(&now, &tm);
  strftime(TimeChar, 80, "%a, %e.%m.%G, %H:%M:%S %Z", &tm);
  Serial.printf("%s\n", TimeChar);
  // set the speed and acceleration
  server.on("/", handleRoot);      //Which routine to handle at root location. This is display page
  server.onNotFound(handleNotFound);
  serverUpdater.setup(&server);
  server.begin();

  homeStepper();
  Serial.println("Geht los!");
  DayStepDemo();
  delay(1000);
}

void loop() {
  server.handleClient();
  time(&now);
  localtime_r(&now, &tm);
  CalcNewPosition();
  if ((tm.tm_hour == 4) && (tm.tm_min == 5) && (tm.tm_sec == 30)) {
    homeStepper();  //Kalibrieren täglich um 4:5:30
    NewData = true;
  }
}
