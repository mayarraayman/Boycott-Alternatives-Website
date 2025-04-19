#include <WiFi.h>
#include "ThingSpeak.h"

const char* ssid = "Mayar";   // your network SSID (name)
const char* password = "AA854620";   // your network password

WiFiClient  client;

unsigned long myChannelNumber = 2;
const char * myWriteAPIKey = "DYZ0X80YYQRR1RJB";

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

#include "DHT.h"
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#include <MQUnifiedsensor.h>
#define placa "ESP32"
#define Voltage_Resolution 3.3
#define pin A4 //Analog input 0 of your arduino
#define type "MQ-135" //MQ135
#define ADC_Bit_Resolution 12 // For arduino UNO/MEGA/NANO
#define RatioMQ135CleanAir 3.6//RS / R0 = 3.6 ppm  
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);

void setup() {

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);  // Initialize ThingSpeak

  dht.begin();

  MQ135.setRegressionMethod(1);
  MQ135.init();
  float calcR0 = 0;
  for (int i = 1; i <= 10; i ++)
  {
    MQ135.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
  }
  MQ135.setR0(calcR0 / 10);
}

void loop() {

  if ((millis() - lastTime) > timerDelay) {
    if (WiFi.status() != WL_CONNECTED) {
      while (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(ssid, password);
        delay(5000);
      }
    }

    float t = dht.readTemperature();

    MQ135.update(); // Update data, the arduino will read the voltage from the analog pin
    MQ135.setA(605.18); MQ135.setB(-3.937); // Configure the equation to calculate CO concentration value
    float CO = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

    MQ135.setA(77.255); MQ135.setB(-3.18); //Configure the equation to calculate Alcohol concentration value
    float Alcohol = MQ135.readSensor(); // SSensor will read PPM concentration using the model, a and b values set previously or from the setup

    MQ135.setA(110.47); MQ135.setB(-2.862); // Configure the equation to calculate CO2 concentration value
    float CO2 = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

    MQ135.setA(44.947); MQ135.setB(-3.445); // Configure the equation to calculate Toluen concentration value
    float Toluen = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

    MQ135.setA(102.2 ); MQ135.setB(-2.473); // Configure the equation to calculate NH4 concentration value
    float NH4 = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

    MQ135.setA(34.668); MQ135.setB(-3.369); // Configure the equation to calculate Aceton concentration value
    float Aceton = MQ135.readSensor();

    ThingSpeak.setField(1, t);
    ThingSpeak.setField(2, CO);
    ThingSpeak.setField(3, Alcohol);
    ThingSpeak.setField(4, CO2);
    ThingSpeak.setField(5, Toluen);
    ThingSpeak.setField(6, NH4);
    ThingSpeak.setField(7, Aceton);

    ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    lastTime = millis();
  }

}
