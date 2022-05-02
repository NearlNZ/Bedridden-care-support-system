/*==============================================================================
                                    Note
================================================================================
Install ESP8266 board to your Arduino IDE first
1. copy this link --> http://arduino.esp8266.com/stable/package_esp8266com_index.json
2. Put link in File > Preferences > Additional Boards Manager URLs
3. open Tools > Board > Boards Manager
4. install "esp8266 3.0.2" 

Select Board
1. open Tools > Board > ESP8266 Boards (3.0.2)
2. select "NodeMCU 1.0 (ESP-12E Module)"

Install this following libraries in your Arduino IDE
- blynk 1.0.1
- TridentTD_Linenotify 3.0.3
================================================================================*/

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TridentTD_LineNotify.h>
#define BLYNK_PRINT Serial

/*==============================================================================
                    Set your connection, Pin, Token Here    
================================================================================*/
char auth[] = "";             //Blynk auth token (get it from your blynk application)           
char ssid[] = "";             //Wifi SSID
char pass[] = "";             //Wifi Password
char server[] = "";           //Blynk local server
int port = 0;                 //Blynk connection port
char LINE_TOKEN[] = "";       //Line notify token

                              //========= ESP8266 Pins =========
int sensor = 16;              //D0 --- Non-contact water liquid sensor
int button = 5;               //D1 --- Push button
int button2 = 4;              //D2 --- Push button
int buzzer = 0;               //D3 --- Active Buzzer
int loopLED = 14;             //D5 --- 5mm LED
int sensorLED = 12;           //D6 --- 5mm LED
int buttonLED = 13;           //D7 --- 5mm LED

                              //====== Blynk virtual Pins ======
WidgetLED buttonLed(V0);      //LED Widget 1
WidgetLED loopLed(V1);        //LED Widget 2
WidgetLED sensorLed(V2);      //LED Widget 3
BlynkGuage = V3;              //Guage Widget
BlynkSlider = V4;             //Slider Widget
BlynkStreaming = V5;          //Streaming Widget
BlynkSegment = V6;            //Segment Button Widget
BlynkTimer timer;
/*==============================================================================*/

int buttonAlert = 0;
int sensorAlert = 0;
int loopAlert = 0;
int acceptAlert = 1;

double runTime = 0;
double loopDuration = 7200;
double nextTimeAlert;
double loopStamp = 0;
int loopTimer;

void setup() {
  Serial.begin(9600);
  pinMode(sensor, INPUT);
  pinMode(button, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(loopLED, OUTPUT);
  pinMode(sensorLED, OUTPUT);
  pinMode(buttonLED, OUTPUT);
  
  Blynk.begin(auth, ssid, pass, server, port);
  Blynk.virtualWrite(BlynkGuage,loopDuration/60);
  Blynk.setProperty(BlynkSlider, "max", loopDuration/60);
  Blynk.virtualWrite(BlynkSegment,1);

  timer.setInterval(500, buttonCheck);
  timer.setInterval(1000, sensorCheck);
  timer.setInterval(1000, dataUpdate);
  timer.setInterval(1000, alert);
  loopTimer = timer.setInterval(loopDuration*1000, loopCheck);
  
  LINE.setToken(LINE_TOKEN);
}

BLYNK_WRITE(BlynkGuage){
  loopDuration = param.asFloat()*60;
  loopStamp = runTime;
  timer.deleteTimer(loopTimer);
  Blynk.setProperty(BlynkSlider, "max", loopDuration/60);
  loopTimer = timer.setInterval(loopDuration*1000, loopCheck);
}

BLYNK_WRITE(BlynkSegment){
  acceptAlert = param.asInt();
}

void dataUpdate(){
  runTime = (millis()/1000);
  nextTimeAlert = ((loopDuration)-(runTime-loopStamp))/60;
  Blynk.virtualWrite(BlynkSlider,nextTimeAlert);
}

void buttonCheck(){
  if(digitalRead(button)==HIGH || digitalRead(button2)==HIGH){
    if (buttonAlert == 0){
      buttonAlert = 1;
      buttonLed.on();
      digitalWrite(buttonLED,HIGH);
      Serial.println("ผู้ป่วยกดปุ่มเรียกฉุกเฉิน กรุณามาที่ห้องผู้ป่วยโดยด่วน");
      Blynk.notify("ผู้ป่วยกดปุ่มเรียกฉุกเฉิน กรุณามาที่ห้องผู้ป่วยโดยด่วน");
      LINE.notify("ผู้ป่วยกดปุ่มเรียกฉุกเฉิน กรุณามาที่ห้องผู้ป่วยโดยด่วน");
    }else{
      buttonAlert = 0;
      loopAlert = 0;
      buttonLed.off();
      digitalWrite(buttonLED,LOW);
      loopLed.off();
      digitalWrite(loopLED,LOW);
    }
  }
}

void loopCheck(){
  loopStamp = runTime;
  if(acceptAlert == 1){
    buttonAlert = 1;
    loopLed.on();
    digitalWrite(loopLED,HIGH);
    loopAlert = 1;
    Serial.println("ถึงเวลาพลิกตัวผู้ป่วย กรุณามาที่ห้องผู้ป่วย");
    Blynk.notify("ถึงเวลาพลิกตัวผู้ป่วย กรุณามาที่ห้องผู้ป่วย"); 
    LINE.notify("ถึงเวลาพลิกตัวผู้ป่วย กรุณามาที่ห้องผู้ป่วย"); 
  }
}

void sensorCheck(){
  if(digitalRead(sensor) == HIGH){
    sensorLed.on();
    digitalWrite(sensorLED,HIGH);
    if(sensorAlert == 0){
      sensorAlert = 1;
      Serial.println("ถุงเก็บปัสสาวะใกล้เต็ม กรุณาเปลี่ยนถุงเก็บปัสสาวะ");
      Blynk.notify("ถุงเก็บปัสสาวะใกล้เต็ม กรุณาเปลี่ยนถุงเก็บปัสสาวะ");
      LINE.notify("ถุงเก็บปัสสาวะใกล้เต็ม กรุณาเปลี่ยนถุงเก็บปัสสาวะ");
    }
  }else{
    sensorAlert = 0;
    sensorLed.off();
    digitalWrite(sensorLED,LOW);
  }
}

void alert(){
  if(sensorAlert == 1 || buttonAlert == 1){
    digitalWrite(buzzer,LOW);
    delay(250);
    digitalWrite(buzzer,HIGH);
  }else{
    digitalWrite(buzzer,HIGH);
  }
}

void loop() {
  Blynk.run();
  timer.run();
}
