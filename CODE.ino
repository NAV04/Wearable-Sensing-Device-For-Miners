 

#include <LiquidCrystal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define heart 13                              

const int rs = 13, en = 12, d4 =14 , d5 = 27, d6 = 26, d7 = 25;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS     1000


#include<dht.h>  
#define dht_dpin 2


dht DHT;

float tempc,Humidity;

#include <Wire.h>

#include <Adafruit_Sensor.h> 

#include <Adafruit_ADXL345_U.h>

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();


#include <WiFi.h>
#include "ThingSpeak.h"

const char* ssid = "USERNAME";   
const char* password = "PASSWORD";  
WiFiClient  client;

unsigned long myChannelNumber = "FROM THINGS SPEAK;
const char * myWriteAPIKey = "FROM THINGS SPEAK";



// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

void Connect_wifi();
  int X_val,Y_val;
int Gas_val;

float BPM, SpO2,BP;

PulseOximeter pox;
uint32_t tsLastReport = 0;
String RFID_buff="";
void Init_spo2();

void onBeatDetected()
{
  Serial.println("Beat Detected!");
}

void setup() 
{
    Serial.begin(9600);  //Initialize serial
    Serial2.begin(9600);
    lcd.begin(16, 2);
    
    lcd.clear();
    lcd.print("Real Time Person");
    lcd.setCursor(0,1);
    lcd.print("Monitoring S/m");
  
  WiFi.mode(WIFI_STA);   
  
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  Connect_wifi();

   Serial.print("Initializing pulse oximeter..");

  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {  
    Serial.println("SUCCESS");

    pox.setOnBeatDetectedCallback(onBeatDetected);
  }

 
   if(!accel.begin())
   {

      Serial.println("No valid sensor found");

      while(1);

   }
 
//  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
    //Connect_wifi();
      pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
    
}

void Connect_wifi()
{



  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Init_spo2();
}
void Init_spo2()
{
   Serial.print("Initializing pulse oximeter..");
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");

    pox.setOnBeatDetectedCallback(onBeatDetected);
  }

  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);



}
void RFID_CHECK()
{
  lcd.clear();
  lcd.print("Place Your Card");
  Serial.println("Place Your Card");
  delay(1000);

    if(Serial2.available()>0)
    {
      RFID_buff=Serial2.readString();
      Serial.println(RFID_buff);
      delay(1000);
  
      if((RFID_buff=="4B00E1D03A40")) 
      {
        lcd.clear();
        lcd.print("Valid Card..");
        Serial.println("Valid Card");
        delay(1000);
      }
      else
      {
         lcd.clear();
        lcd.print("Invalid Card..");
           Serial.println("Invalid Card");
        delay(1000);
      }
    }
 
}
void loop() 
{
   int i=0;
   Init_spo2();
    while(i<10000)
    {
        pox.update();
        BP = pox.getHeartRate();
        SpO2 = pox.getSpO2();
      
        if (millis() - tsLastReport > REPORTING_PERIOD_MS)
        {
//      
          Serial.print("HB: ");
          Serial.println(BP);
      
          Serial.print("SpO2: ");
          Serial.print(SpO2);
          Serial.println("%");
          lcd.clear();
          lcd.print("HB:");
          lcd.print(BP); 
          lcd.setCursor(0,1);
          lcd.print("Spo2:");
          lcd.print(SpO2);   // print the temperature in °C
          lcd.print("%");
      
          Serial.println("*********************************");
          Serial.println();
      
          tsLastReport = millis();
        }
//        delay(500);
        i++;
    }
  RFID_CHECK();
  Temp_check();
  Fall_check();
  Gas_Check();
 

    ThingSpeak.setField(1, tempc);
    ThingSpeak.setField(2, Humidity);
    ThingSpeak.setField(4, X_val);
      ThingSpeak.setField(5, Y_val);
    ThingSpeak.setField(3, Gas_val);
     ThingSpeak.setField(6, BP);
    

    // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
    // pieces of information in a channel.  Here, we write to field 1.
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    if(x == 200){
      Serial.println("Channel update successful.");
      Connect_wifi();

    }
    else{
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    lastTime = millis();
} 

void Temp_check()
{
     DHT.read11(dht_dpin);
      Humidity=DHT.humidity;
      tempc=DHT.temperature;
     Serial.println("temp="+String(tempc));
     Serial.println("Humidity="+String(Humidity));
     lcd.clear();
     lcd.print("temp="+String(tempc));
     lcd.setCursor(0,1);
     lcd.print("Humidity="+String(Humidity));
     delay(1000);

}
void Gas_Check()
{

  Gas_val= analogRead(36);

  Serial.print("Gas: ");
  Serial.println(Gas_val);
  delay(1000);


}
void Fall_check() 
{
  sensors_event_t event; 

   accel.getEvent(&event);

   X_val=event.acceleration.x;

   Y_val=event.acceleration.y;

  int Z_val=event.acceleration.z;
   

   Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print("  ");

   Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print("  ");

   //Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print("  ");

   Serial.println("m/s^2 ");

   lcd.clear();
   lcd.print("X:");
   lcd.print(X_val);
   lcd.setCursor(0,1);
   lcd.print("Y:");
   lcd.print(Y_val);
 

   delay(500);

}
