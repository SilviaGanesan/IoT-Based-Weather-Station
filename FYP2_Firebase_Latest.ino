#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <SPI.h>
#include <LoRa.h>
#include "time.h"


// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#define WIFI_SSID "Silvia"
#define WIFI_PASSWORD "Silvia0946"


// Insert Firebase project API Key
#define API_KEY "AIzaSyCi3kAbQHMutZ_zER4Cuht1zp2-GvjNB9c"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://fyptest-92889-default-rtdb.asia-southeast1.firebasedatabase.app/" 

// LoRa configuration
#define SS 5
#define RST 26 
#define DI0 2
#define BAND 433E6

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;


// Variables to store sensor data
String device_id;
String temperature;
String pressure;
String altitude;
String humidity;
String dewPoint;
String rainfall;

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // LoRa Setup (with Pin Assignments and Reset)
  SPI.begin(18, 19, 23, 5); // SCK, MISO, MOSI, SS (NSS on GPIO 5)
  LoRa.setPins(SS, RST, DI0);

  // Manual LoRa Module Reset
  pinMode(RST, OUTPUT);
  digitalWrite(RST, LOW);
  delay(10);
  digitalWrite(RST, HIGH);
  delay(100);

  // Check LoRa Initialization with Detailed Logging
  Serial.println("Initializing LoRa...");
  while (!LoRa.begin(BAND)) {
    Serial.println("LoRa failed. Retrying...");
    delay(500);
  }

  LoRa.setSyncWord(0x0B);
  Serial.println("LoRa Initializing OK!");
}


void loop() {
  // Try to parse packet
  int pos1, pos2, pos3, pos4, pos5, pos6; // Removed pos7
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.println();
    Serial.println("============================================================");
    Serial.print("Received packet: ");
    String LoRaData = LoRa.readString();
    Serial.print(LoRaData);

    // Extract data from packet
    pos1 = LoRaData.indexOf('/');
    pos2 = LoRaData.indexOf('&');
    pos3 = LoRaData.indexOf('#');
    pos4 = LoRaData.indexOf('@');
    pos5 = LoRaData.indexOf('$');
    pos6 = LoRaData.indexOf('^');
    
   // Ensure all delimiters are found
    if (pos1 != -1 && pos2 != -1 && pos3 != -1 && pos4 != -1 && pos5 != -1 && pos6 != -1) {
        // Extract individual sensor data
    device_id = LoRaData.substring(0, pos1);
    temperature = LoRaData.substring(pos1 + 1, pos2);
    pressure = LoRaData.substring(pos2 + 1, pos3);
    altitude = LoRaData.substring(pos3 + 1, pos4);
    humidity = LoRaData.substring(pos4 + 1, pos5);
    dewPoint = LoRaData.substring(pos5 + 1, pos6);
    rainfall = LoRaData.substring(pos6 + 1);


      Serial.println("Device ID: " + device_id);
      Serial.println("Temperature: " + temperature + " °C");
      Serial.println("Pressure: " + pressure + " hPa");
      Serial.println("Altitude: " + altitude + " m");
      Serial.println("Humidity: " + humidity + " %");
      Serial.println("Dew Point: " + dewPoint + " °C");
      Serial.println("Rainfall: " + rainfall + " %");
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    // Write an Int number on the database path test/int
    if (Firebase.RTDB.setString(&fbdo, "test/temperature",(temperature))){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    
    // Write an Float number on the database path test/float
    if (Firebase.RTDB.setString(&fbdo, "test/pressure", (pressure))){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setString(&fbdo, "test/altitude", (altitude))){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setString(&fbdo, "test/humidity", (humidity))){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  
     if (Firebase.RTDB.setString(&fbdo, "test/dewPoint", (dewPoint))){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setString(&fbdo, "test/rainfall", (rainfall))){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    

} //end of firebase one 


    } 
    
    else {
      Serial.println("Incomplete LoRa packet received, missing delimiters");
    }
  }
}
