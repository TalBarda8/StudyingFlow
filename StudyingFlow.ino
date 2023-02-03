/*
 * "StudyingFlow" is a cutting-edge app designed to streamline your studying experience.
 * It seamlessly integrates with a CPX device to send messages between the two via sensors or dashboard controllers.
 * 
 * When the Study Mode button is turned on, the device label displays "Keep studying! It's your time to shine!", and the CPX LED lights turn red, signaling the start of your study period.
 * The timer begins counting the elapsed time for your current session.
 * 
 * If you need a break, simply move the Break Timer slide to the desired number of minutes. 
 * The device and CPX display the remaining time until the end of your break. When the break is over, the label announces it and the CPX lights gradually fade into red. 
 * To end the break, simply cover the light sensor until its value falls below 30. A new study session will then commence.
 * 
 * After an hour of continuous study, the label will suggest taking a break to refresh and recharge. 
 * 
 * This innovative app was created by Tal Barda and Bar Efrima to enhance your learning experience. 
 *  
*/

/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID           "TMPLMJ5umJPN"
#define BLYNK_DEVICE_NAME           "My CPX Instance"
#define BLYNK_AUTH_TOKEN            "9h6QMEln2bDnIRPqdBwWBk0aR6-cu0_b"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT SerialUSB

#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>
#include <Adafruit_CircuitPlayground.h>

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "";
char pass[] = "";

// Hardware Serial on Mega, Leonardo, Micro...
#define EspSerial Serial1

// or Software Serial on Uno, Nano...
//#include <SoftwareSerial.h>
//SoftwareSerial EspSerial(2, 3); // RX, TX

// Your ESP8266 baud rate:
#define ESP8266_BAUD 115200

ESP8266 wifi(&EspSerial);

void setup()
{
  CircuitPlayground.begin();
  SerialUSB.begin(115200);
  EspSerial.begin(ESP8266_BAUD);
  delay(10);
  Blynk.begin(BLYNK_AUTH_TOKEN, wifi, ssid, pass);
}

unsigned long lastSent;
unsigned long lastSentbreak;
int brightness = 0;  
int fadeAmount = 5;
bool firstLoop = false;
bool wereOnBreak = false;
int pinValue = 0;
int secondsStuding = 0;
int minutesStuding = 0;
int hoursStuding = 0;
int lightLevel;

void loop()
{
  Blynk.run();
  
  //Studing mode is ON
  if (digitalRead(13) == HIGH) {  
    
    if(millis() - lastSent >= 1000)
    {
      //Resetting the CPX when start studying session.
      if (firstLoop)              
      {
        turnOffPixels();
        lastSent = millis();
        firstLoop = false;
      }
      
      secondsStuding = secondsStuding + (millis() - lastSent)/300;
      studyingTimeDisplay();

      // When taking a break, the pinValue value setted to the number of minutes of the break.
      while (pinValue > 0)
      {
        CircuitPlayground.setPixelColor(pinValue, 0, 0, 0);
        if(millis() - lastSentbreak >= 60*1000)
        {
          pinValue = pinValue - 1;
          lastSentbreak = millis();
        }
        Blynk.virtualWrite(V0, pinValue);
        
        lightLevel = CircuitPlayground.lightSensor();
        Blynk.virtualWrite(V1, lightLevel);

        //Break is over, entering the while loop until lightLevel<=30.
        while (pinValue == 0 && lightLevel > 30)
        {
          lightLevel = CircuitPlayground.lightSensor();
          Blynk.virtualWrite(V1, lightLevel);
          Blynk.virtualWrite(V5, "Break is over!");
          
          for (int i = 0; i < 10; i++) 
          {
            CircuitPlayground.setPixelColor(i, brightness, 0, 0);
            brightness = brightness + fadeAmount;
      
            if (brightness <= 0 || brightness >= 255)
            {
              fadeAmount = -fadeAmount;
            }
          }
        }
         wereOnBreak = true;
      }

      //Reseting the lights and time period after the break is done.
      if (wereOnBreak)
      {
        turnOffPixels();
        wereOnBreak = false;
        setTimeToZero();
      }
      
      lastSent = millis();
    }
  }
  else //Studing mode is OFF
  {
      firstLoop = true;
      setTimeToZero();     
      Blynk.virtualWrite(V5, "Who's ready to study?!");
      for (int i = 0; i < 10; i++) 
      {
        CircuitPlayground.setPixelColor(i, 255, 255, 255);
      }
      delay(200);
    }
}

BLYNK_WRITE(V0) {
  pinValue = param.asInt(); // assigning incoming value from pin V0 to a variable
  Serial.println(pinValue);
  
  for (int i = 0; i < pinValue; ++i) {
    CircuitPlayground.setPixelColor(i, 255, 0, 255);
  }

  for (int i = pinValue; i < 10; ++i) {
    CircuitPlayground.setPixelColor(i, 0, 0, 0);
  }
}

//Resetting time period to 0.
void setTimeToZero()
{
  secondsStuding = 0;
  minutesStuding = 0;
  hoursStuding = 0;
  Blynk.virtualWrite(V2, secondsStuding);
  Blynk.virtualWrite(V3, minutesStuding);
  Blynk.virtualWrite(V4, hoursStuding);
}

//Updating the minues and hours values if needed.
void studyingTimeDisplay()
{
  if (secondsStuding >= 60)
      {
        secondsStuding = 0;
        minutesStuding = minutesStuding + 1;

        if (minutesStuding == 60)
        {
          minutesStuding = 0;
          hoursStuding = hoursStuding + 1;
        }
      }
      
      Blynk.virtualWrite(V2, secondsStuding);
      Blynk.virtualWrite(V3, minutesStuding);
      Blynk.virtualWrite(V4, hoursStuding);

      if (hoursStuding > 0)
      {
        Blynk.virtualWrite(V5, "It's time for a break!");
      }
      else
      {
        Blynk.virtualWrite(V5, "Keep studing! It's your time to shine!");
      }
}

void turnOffPixels()
{
  for (int i = 0; i < 10; ++i) 
  {
    CircuitPlayground.setPixelColor(i, 0, 0, 0);
  }
}
