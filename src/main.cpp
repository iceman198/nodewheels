#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WebSocketsServer.h>

#include "index.h"

//Hotspot Name and Password
const char *ssid = "dwheels";
const char *password = "1234567890";

WebSocketsServer webSocket = WebSocketsServer(8181);
ESP8266WebServer server(80);

int ledPin = 16; //0 is the built in RED LED on the NodeMCU...but when we use the arduino libraries, the pin numbers are different so it is 16 in this case
bool ledOn = false;
int mycounter = 0;
int webSendDelay = 2000;
bool clientConnected = false;
bool voltageGood = true;
int voltageCount = 0;
bool serialMsgSent = true;

float goodVoltage = 9.00;

// 15 works on 3.3v
// 10, 12, 13, 14 work when on ground
int RIGHT_F_PIN = 12; // D6
int RIGHT_R_PIN = 14; // D5

int LEFT_F_PIN = 10; // SD3
int LEFT_R_PIN = 13; // D7

//int EN_PIN = 13;

// Think of it as you're sitting in the machine and looking to your left or right
int RF_PWM = 2; // Right engine 'Forward' speed
int RR_PWM = 0; // Right engine 'Reverse' speed
int LF_PWM = 4; // Left engine 'Forward' speed
int LR_PWM = 5; // Left engine 'Reverse' speed

int rf_current = 0;
int rr_current = 0;
int rf_dest = 0;
int rr_dest = 0;
int lf_current = 0;
int lr_current = 0;
int lf_dest = 0;
int lr_dest = 0;

//int max_speed = 512;
//int max_speed = 767;
int fastSpeed = 900;
int mediumSpeed = 600;
int slowSpeed = 300;
int max_speed = mediumSpeed;

int delaySpeed = 2;
//int delaySpeed = 1000;

void handleRoot()
{
  //Serial.println("handleRoot() ~ start");
  String html = MAIN_page;
  server.send(200, "text/html", html);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, int length)
{
  if (type == WStype_TEXT)
  {
    Serial.println("webSocketEvent() ~ incoming payload");
    Serial.print("webSocketEvent() ~ num = ");
    Serial.println(num);
    String fullpayload = "";
    for (int i = 0; i < length; i++)
    {
      //Serial.print((char)payload[i]);
      fullpayload += (char)payload[i];
    }
    Serial.print("webSocketEvent() ~ fullpayload = ");
    Serial.println(fullpayload);
    Serial.println("webSocketEvent() ~ payload complete");
    Serial.println("");
    if (fullpayload == "speedFast") {
      max_speed = fastSpeed;
    }
    if (fullpayload == "speedMedium") {
      max_speed = mediumSpeed;
    }
    if (fullpayload == "speedSlow") {
      max_speed = slowSpeed;
    }
    if (fullpayload == "goForward")
    {
      rf_dest = max_speed;
      rr_dest = 0;

      lf_dest = max_speed;
      lr_dest = 0;
    }
    if (fullpayload == "goStop")
    {
      rf_dest = 0;
      rr_dest = 0;

      lf_dest = 0;
      lr_dest = 0;
    }
    if (fullpayload == "goLeft")
    {
      rf_dest = max_speed;
      rr_dest = 0;

      lf_dest = 0;
      lr_dest = 0;
    }
    if (fullpayload == "goRight")
    {
      rf_dest = 0;
      rr_dest = 0;

      lf_dest = max_speed;
      lr_dest = 0;
    }
    if (fullpayload == "goBackward")
    {
      rf_dest = 0;
      //rr_dest = (max_speed / 2) * -1;
      rr_dest = max_speed;

      lf_dest = 0;
      //lr_dest = (max_speed / 2) * -1;
      lr_dest = max_speed;
    }
  }
  if (type == WStype_CONNECTED)
  {
    clientConnected = true;
      rf_dest = 0;
      rr_dest = 0;

      lf_dest = 0;
      lr_dest = 0;
    Serial.println("webSocketEvent() ~ Client connected");
  }
  if (type == WStype_DISCONNECTED)
  {
    clientConnected = false;
    Serial.println("webSocketEvent() ~ Client disconnected");
  }
  if (type == WStype_ERROR)
  {
    Serial.println("webSocketEvent() ~ Client ERROR");
  }
  if (type == WStype_BIN)
  {
    Serial.println("webSocketEvent() ~ Client WStype_BIN");
  }
  if (type == WStype_FRAGMENT_TEXT_START)
  {
    Serial.println("webSocketEvent() ~ Client WStype_FRAGMENT_TEXT_START");
  }
  if (type == WStype_FRAGMENT_BIN_START)
  {
    Serial.println("webSocketEvent() ~ Client WStype_FRAGMENT_BIN_START");
  }
  if (type == WStype_FRAGMENT)
  {
    Serial.println("webSocketEvent() ~ Client WStype_FRAGMENT");
  }
  if (type == WStype_FRAGMENT_FIN)
  {
    Serial.println("webSocketEvent() ~ Client WStype_FRAGMENT_FIN");
  }
}

void readVoltage() {
  int v = analogRead(A0);
  float v2 = v / 6.56;
  float v3 = v2 / 10;

  String s = "VOLTAGE=";
  String msg = s + v3;
  String s3 = " | analogVal=";
  s3 = s3 + v;
  msg = msg + s3;
  Serial.println(msg);

  if (v3 > goodVoltage) {
    voltageGood = true;
    voltageCount = 0;
    Serial.println("Voltage is good");
  } else {
    voltageCount++;
    if (voltageCount > 3) {
      voltageGood = false;
      Serial.println("Voltage flagged as BAAAD");
    }
    Serial.println("Voltage is BAAAD");
  }

  if (clientConnected) {
    // 800 = 12.36v
    // 74 = 1.167v
    webSocket.sendTXT(0, msg);
  }
}

void myLedLoop()
{
  //for some reason it is opposite...when I set it LOW the LED comes on...is it opposite day?
  //Serial.print("myLedLoop() ~ ledOn: ");
  //Serial.println(ledOn);

  if (ledOn)
  {
    digitalWrite(ledPin, HIGH);
    ledOn = false;
  }
  else
  {
    digitalWrite(ledPin, LOW);
    ledOn = true;
  }
}

void printSpeedInfo()
{
  /*
  Serial.print("printSpeedInfo() ~ l_current: ");
  Serial.print(l_current);
  Serial.print(" | l_dest: ");
  Serial.print(l_dest);
  Serial.print(" | r_current: ");
  Serial.print(r_current);
  Serial.print(" | r_dest: ");
  Serial.print(r_dest);
  Serial.println("");
  */
}

void checkButtons()
{
  /*
  Serial.print("RIGHT_F_PIN (");
  Serial.print(RIGHT_F_PIN);
  Serial.print(") = ");
  Serial.println(digitalRead(RIGHT_F_PIN));
  Serial.print("RIGHT_R_PIN (");
  Serial.print(RIGHT_R_PIN);
  Serial.print(") = ");
  Serial.println(digitalRead(RIGHT_R_PIN));

  Serial.print("LEFT_F_PIN (");
  Serial.print(LEFT_F_PIN);
  Serial.print(") = ");
  Serial.println(digitalRead(LEFT_F_PIN));
  Serial.print("LEFT_R_PIN (");
  Serial.print(LEFT_R_PIN);
  Serial.print(") = ");
  Serial.println(digitalRead(LEFT_R_PIN));
  */

  if (digitalRead(LEFT_R_PIN) == 0) {
    lr_dest = max_speed;
  }
  if (digitalRead(LEFT_R_PIN) == 1) {
    lr_dest = 0;
  }
  if (digitalRead(LEFT_F_PIN) == 0) {
    lf_dest = max_speed;
  }
  if (digitalRead(LEFT_F_PIN) == 1) {
    lf_dest = 0;
  }

  if (digitalRead(RIGHT_R_PIN) == 0) {
    rr_dest = max_speed;
  }
  if (digitalRead(RIGHT_R_PIN) == 1) {
    rr_dest = 0;
  }
  if (digitalRead(RIGHT_F_PIN) == 0) {
    rf_dest = max_speed;
  }
  if (digitalRead(RIGHT_F_PIN) == 1) {
    rf_dest = 0;
  }

  //delay(1000);
}

void modSpeed()
{
  // set the right engine
  if (rr_dest > rr_current)
  {
    rr_current++;
    printSpeedInfo();
    myLedLoop();
    analogWrite(RR_PWM, rr_current);
  }
  if (rr_dest < rr_current)
  {
    rr_current--;
    printSpeedInfo();
    myLedLoop();
    analogWrite(RR_PWM, rr_current);
  }
  if (rf_dest > rf_current)
  {
    rf_current++;
    printSpeedInfo();
    myLedLoop();
    analogWrite(RF_PWM, rf_current);
  }
  if (rf_dest < rf_current)
  {
    rf_current--;
    printSpeedInfo();
    myLedLoop();
    analogWrite(RF_PWM, rf_current);
  }

  // set the left engine
  if (lr_dest > lr_current)
  {
    lr_current++;
    printSpeedInfo();
    myLedLoop();
    analogWrite(LR_PWM, lr_current);
  }
  if (lr_dest < lr_current)
  {
    lr_current--;
    printSpeedInfo();
    myLedLoop();
    analogWrite(LR_PWM, lr_current);
  }
  if (lf_dest > lf_current)
  {
    lf_current++;
    printSpeedInfo();
    myLedLoop();
    analogWrite(LF_PWM, lf_current);
  }
  if (lf_dest < lf_current)
  {
    lf_current--;
    printSpeedInfo();
    myLedLoop();
    analogWrite(LF_PWM, lf_current);
  }

  // check if we are stopped
  if (lr_dest == 0 && lr_current == 0 && lf_dest == 0 && lf_current == 0)
  {
    analogWrite(LR_PWM, lr_current);
    analogWrite(LF_PWM, lf_current);
    //digitalWrite(L_EN, LOW);
  }
  else
  {
    //digitalWrite(L_EN, HIGH);
  }
  if (rr_dest == 0 && rr_current == 0 && rf_dest == 0 && rf_current == 0)
  {
    analogWrite(RR_PWM, rr_current);
    analogWrite(RF_PWM, rf_current);
    //digitalWrite(R_EN, LOW);
  }
  else
  {
    //digitalWrite(R_EN, HIGH);
  }
}

void setup()
{
  delay(1000);
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  //Setup Buttons
  pinMode(RIGHT_F_PIN, INPUT_PULLUP);
  pinMode(RIGHT_R_PIN, INPUT_PULLUP);
  pinMode(LEFT_F_PIN, INPUT_PULLUP);
  pinMode(LEFT_R_PIN, INPUT_PULLUP);

  //Setup voltage read
  pinMode(A0, INPUT);

  //Setup motors
  //pinMode(EN_PIN, OUTPUT);
  pinMode(RR_PWM, OUTPUT);
  pinMode(RF_PWM, OUTPUT);
  pinMode(LR_PWM, OUTPUT);
  pinMode(LF_PWM, OUTPUT);

  //digitalWrite(EN_PIN, HIGH);

  //Setup WIFI
  Serial.println("setup() ~ Configuring access point...");
  //WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  if (MDNS.begin("esp8266"))
  {
    Serial.println("setup() ~ MDNS responder started");
  }
  server.on("/", handleRoot);
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  Serial.println("setup() ~ HTTP server started");
  Serial.print("setup() ~ HotSpot IP:");
  Serial.println(myIP);
}

void loop()
{
  server.handleClient();
  webSocket.loop();

  if (mycounter > webSendDelay) {
    myLedLoop();
    readVoltage();
    mycounter = 0;
  }
  mycounter++;

  if (!clientConnected) {
    checkButtons();
  }
  if (!voltageGood) {
    rf_dest = 0;
    rr_dest = 0;

    lf_dest = 0;
    lr_dest = 0;
  }
  modSpeed();

  delay(delaySpeed);
}
