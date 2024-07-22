#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>

#define BAND 433E6
#define rain_sensor A0
#define SEALEVELPRESSURE_HPA (742.45)


Adafruit_BMP280 bmp;
DHT dht(4, DHT22); // Specify DHT22 sensor type and pin
String LoRaMessage = "";
char device_id[12] = "MyDevice123";

void setup() {
  Serial.begin(2000000); // Lower baud rate for compatibility with LoRa
  Wire.begin();
  dht.begin();
  pinMode(rain_sensor, INPUT);
  while (!Serial);
  Serial.println(F("LoRa Sender"));
  if (!LoRa.begin(BAND)) {
    Serial.println(F("Starting LoRa failed!"));
    while (1);
  }
  LoRa.setSyncWord(0x0B);
  Serial.println("LoRa Initializing OK!");

  if (!bmp.begin(0x76)) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }
}

void loop() {
  float pressure = bmp.readPressure() / 100.0F;
  float altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA);

  float humidity_dht = dht.readHumidity();
  float temperature_dht = dht.readTemperature();
  double dewPoint = dewPointFast(temperature_dht, humidity_dht);
  int rainfall = map(analogRead(rain_sensor), 780, 0, 0, 100);
  

  if (rainfall >= 100) {
   rainfall = 100;
  }
  if (rainfall <= 0) {
   rainfall = 0;
  }

  Serial.print(F("Device ID: "));
  Serial.print(device_id);
  Serial.println();
  Serial.print(F("Temperature = "));
  Serial.print(temperature_dht);
  Serial.print(F(" *C"));
  Serial.println();
  Serial.print(F("Pressure = "));
  Serial.print(pressure);
  Serial.print(F(" hPa"));
  Serial.println();
  Serial.print(F("Approx. Altitude = "));
  Serial.print(altitude);
  Serial.print(F(" m"));
  Serial.println();
  Serial.print(F("Humidity = "));
  Serial.print(humidity_dht);
  Serial.print(F("%"));
  Serial.println();
  Serial.print(F("Dew point = "));
  Serial.print(dewPoint);
  Serial.print(F(" *C"));
  Serial.println();
  Serial.print(F("Rainfall = "));
  Serial.print(rainfall);
  Serial.print(F("%"));
  Serial.println();
  Serial.println("===========================================");
  Serial.println();
  Serial.println();

  LoRaMessage = String(device_id) + "/" + String(temperature_dht) + "&" +
               String(pressure) + "#" + String(altitude) + "@" + String(humidity_dht) +
               "$" + String(dewPoint) +
               "^" + String(rainfall);

  // send packet
  LoRa.beginPacket();
  LoRa.print(LoRaMessage);
  LoRa.endPacket();
  
  delay(10000);
}

double dewPointFast(double celsius, double humidity) {
  double a = 17.271;
  double b = 237.7;
  double temp = (a * celsius) / (b + celsius) + log(humidity * 0.01);
  double Td = (b * temp) / (a - temp);
  return Td;
}
