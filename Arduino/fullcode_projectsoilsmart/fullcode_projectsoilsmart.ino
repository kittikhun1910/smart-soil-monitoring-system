/**
 * Created by K. Suwatchai (Mobizt)
 * 
 * Email: k_suwatchai@hotmail.com
 * 
 * Github: https://github.com/mobizt
 * 
 * Copyright (c) 2021 mobizt
 *
*/

#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#endif

// include ไลบรารีสำหรับรับค่าเวลา
#include <NTPClient.h>
#include <WiFiUdp.h>

// ประกาศฟังก์ชันสำหรับรับค่าเวลา
// Variable to save current epoch time
unsigned long epochTime; 
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Function that gets current epoch time
unsigned long getTime() {
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  return now;
}
/*นำคำสั่งใช้งานไลบรารี่เซ็นเซอ*/
int sensor_pin = A0;  /* Soil moisture sensor O/P pin */

#include "DHT.h"

#define DHTPIN D2 // what digital pin we're connected to
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

//Provide the token generation process info.
#include <addons/TokenHelper.h>

//Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>


/* 1. Define the WiFi credentials */
#define WIFI_SSID "P"
#define WIFI_PASSWORD "00000000"

//For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "AIzaSyBjrPJOwH2YJ-aAIvDdJf1PcY6_eT1Hqtc"

/* 3. Define the RTDB URL */
#define DATABASE_URL "soil-monitoring-system-961a3-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "6310301008@cdti.ac.th"
#define USER_PASSWORD "1608200001221"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;

void setup()
{
  // สำหรับรับค่าเวลา
  timeClient.begin();
  Serial.begin(115200);
  Serial.println("DHTxx test!");
  // สำหรับรับค่า random
  randomSeed(analogRead(0));

dht.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  //Or use legacy authenticate method
  //config.database_url = DATABASE_URL;
  //config.signer.tokens.legacy_token = "<database secret>";

  //////////////////////////////////////////////////////////////////////////////////////////////
  //Please make sure the device free Heap is not lower than 80 k for ESP32 and 10 k for ESP8266,
  //otherwise the SSL connection will fail.
  //////////////////////////////////////////////////////////////////////////////////////////////

  Firebase.begin(&config, &auth);

  //Comment or pass false value when WiFi reconnection will control by your code or third party library
  Firebase.reconnectWiFi(true);

  Firebase.setDoubleDigits(5);

  /** Timeout options.

  //WiFi reconnect timeout (interval) in ms (10 sec - 5 min) when WiFi disconnected.
  config.timeout.wifiReconnect = 10 * 1000;

  //Socket connection and SSL handshake timeout in ms (1 sec - 1 min).
  config.timeout.socketConnection = 10 * 1000;

  //Server response read timeout in ms (1 sec - 1 min).
  config.timeout.serverResponse = 10 * 1000;

  //RTDB Stream keep-alive timeout in ms (20 sec - 2 min) when no server's keep-alive event data received.
  config.timeout.rtdbKeepAlive = 45 * 1000;

  //RTDB Stream reconnect timeout (interval) in ms (1 sec - 1 min) when RTDB Stream closed and want to resume.
  config.timeout.rtdbStreamReconnect = 1 * 1000;

  //RTDB Stream error notification timeout (interval) in ms (3 sec - 30 sec). It determines how often the readStream
  //will return false (error) when it called repeatedly in loop.
  config.timeout.rtdbStreamError = 3 * 1000;

  Note:
  The function that starting the new TCP session i.e. first time server connection or previous session was closed, the function won't exit until the 
  time of config.timeout.socketConnection.

  You can also set the TCP data sending retry with
  config.tcp_data_sending_retry = 1;

  */

  // random int set-up
  randomSeed(analogRead(0));
}

void loop()
{
  //Flash string (PROGMEM and  (FPSTR), String,, String C/C++ string, const char, char array, string literal are supported
  //in all Firebase and FirebaseJson functions, unless F() macro is not supported.

  if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    



 float moisture_percentage;
    int sensor_analog;
    sensor_analog = analogRead(sensor_pin);
    moisture_percentage = ( 100 - ( (sensor_analog/1023.00) * 100 ) );
    Serial.print("Moisture Percentage = ");
    Serial.print(moisture_percentage);
    Serial.print("%\n\n");
  
      // ให้เซนเซอร์ทำงาน วัดอุณหภูมิ กับความชื้น
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float f = dht.readTemperature(true);
    
    // เช็คถ้าอ่านค่าไม่สำเร็จให้เริ่มอ่านใหม่
    if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
    }
    
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" *C ");
    Serial.print(f);
    Serial.print(" *F\t\n");

    // สำหรับรับค่าเวลา Timestamp Thailand Timestamp GMT:+07.00
    epochTime = getTime();
    Serial.print("Epoch Time: ");
    Serial.println(epochTime);

        // random ph begin 0 - 14 
   int int_ph = random(5,8);
   float flo_ph = random(35, 100);
   float cov_ph = flo_ph /100;
   float ph = int_ph + cov_ph;


   // random Nitrogen  (0 - 200 +-) มก./กก. = มิลลิกรัมต่อกิโลกรัม = มก./1000 กรัม (เท่ากับ 1 ppm) 
   //mg/kg = milligram per kilogram = mg/1000 grams (same as 1 ppm)

   int int_N = random(20,200);
   float flo_N = random(0, 100);
   float cov_N = flo_N /100;
   float nitrogen = int_N + cov_N;

   // random Phosphorus   (0 - 14 +-) มก./กก. = มิลลิกรัมต่อกิโลกรัม = มก./1000 กรัม (เท่ากับ 1 ppm) 
   //mg/kg = milligram per kilogram = mg/1000 grams (same as 1 ppm)

   int int_P = random(3,14);
   float flo_P = random(0, 100);
   float cov_P = flo_P /100;
   float phosphorus  = int_P + cov_P;

   // random Potassium   (0 - 200 +-) มก./กก. = มิลลิกรัมต่อกิโลกรัม = มก./1000 กรัม (เท่ากับ 1 ppm) 
   //mg/kg = milligram per kilogram = mg/1000 grams (same as 1 ppm)

   int int_K = random(20,200);
   float flo_K = random(0, 100);
   float cov_K = flo_K /100;
   float potassium  = int_K + cov_K;

  
    //เเสดงค่า pH
    Serial.print("pH = ");
    Serial.print(ph);
    Serial.print(" pH\n");
    
    //เเสดงค่า Nitogen
    Serial.print("Nitogen = ");
    Serial.print(nitrogen);
    Serial.print(" PPM\n");
    
    //เเสดงค่า Phosphorus
    Serial.print("Phosphorus = ");
    Serial.print(phosphorus);
    Serial.print(" PPM\n");

    //เเสดงค่า Potassium
    Serial.print("Potassium = ");
    Serial.print(potassium);
    Serial.print(" PPM\n\n");
    
    // ส่งและรับค่า temperature C
    Serial.printf("Set  Temperature C... %s\n", Firebase.setFloat(fbdo, "/Set/TemperatureC", t) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Get  Temperature C... %s\n", Firebase.getFloat(fbdo, "/Set/TemperatureC") ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
    int iVal = 0;
    Serial.printf("Get  Temperature C ref... %s\n", Firebase.getFloat(fbdo, "//Set/TemperatureC", &iVal) ? String(iVal).c_str() : fbdo.errorReason().c_str());
    Serial.printf("Push  Temperature C... %s\n", Firebase.pushFloat(fbdo, "/Push/TemperatureC", t) ? "ok" : fbdo.errorReason().c_str());

    // ส่งและรับค่า Humidity
    Serial.printf("Set  Humidity... %s\n", Firebase.setFloat(fbdo, "/Set/Humidity", h) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Get  Humidity... %s\n", Firebase.getFloat(fbdo, "/Set/Humidity") ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
    int iVal_1 = 0;
    Serial.printf("Get  Humidity ref... %s\n", Firebase.getFloat(fbdo, "//Set/Humidity", &iVal_1) ? String(iVal_1).c_str() : fbdo.errorReason().c_str());
    Serial.printf("Push  Humidity... %s\n", Firebase.pushFloat(fbdo, "/Push/Humidity", h) ? "ok" : fbdo.errorReason().c_str());
    

    // ส่งและรับค่า moisture
    Serial.printf("Set  Moisture... %s\n", Firebase.setFloat(fbdo, "/Set/Moisture", moisture_percentage) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Get  Moisture... %s\n", Firebase.getFloat(fbdo, "/Set/Moisture") ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
    int iVal_2 = 0;
    Serial.printf("Get  Moisture ref... %s\n", Firebase.getFloat(fbdo, "//Set/Moisture", &iVal_2) ? String(iVal_2).c_str() : fbdo.errorReason().c_str());
    Serial.printf("Push  Moisture... %s\n", Firebase.pushFloat(fbdo, "/Push/Moisture", moisture_percentage) ? "ok" : fbdo.errorReason().c_str());

    // ส่งและรับค่า temperature F
    Serial.printf("Set  Temperature F... %s\n", Firebase.setFloat(fbdo, "/Set/TemperatureF", f) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Get  Temperature F... %s\n", Firebase.getFloat(fbdo, "/Set/TemperatureF") ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
    int iVal_3 = 0;
    Serial.printf("Get  Temperature ref... %s\n", Firebase.getFloat(fbdo, "//Set/TemperatureF", &iVal_3) ? String(iVal).c_str() : fbdo.errorReason().c_str());
    Serial.printf("Push  Temperature F... %s\n\n", Firebase.pushFloat(fbdo, "/Push/TemperatureF", f) ? "ok" : fbdo.errorReason().c_str());


    // ส่งและรับค่า pH
    Serial.printf("Set pH... %s\n", Firebase.setFloat(fbdo, "/Set/pH", ph) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Get pH... %s\n", Firebase.getFloat(fbdo, "/Set/pH") ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
    int iVal_4 = 0;
    Serial.printf("Get pH ref... %s\n", Firebase.getFloat(fbdo, "//Set/pH", &iVal_4) ? String(iVal).c_str() : fbdo.errorReason().c_str());
    Serial.printf("Push PH... %s\n\n", Firebase.pushFloat(fbdo, "/Push/pH", ph) ? "ok" : fbdo.errorReason().c_str());

    // ส่งและรับค่า Nitrogen
    Serial.printf("Set Nitrogen... %s\n", Firebase.setFloat(fbdo, "/Set/Nitrogen", nitrogen) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Get Nitrogen... %s\n", Firebase.getFloat(fbdo, "/Set/Nitrogen") ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
    int iVal_5 = 0;
    Serial.printf("Get Nitrogen ref... %s\n", Firebase.getFloat(fbdo, "//Set/Nitrogen", &iVal_5) ? String(iVal).c_str() : fbdo.errorReason().c_str());
    Serial.printf("Push Nitrogen... %s\n\n", Firebase.pushFloat(fbdo, "/Push/Nitrogen", nitrogen) ? "ok" : fbdo.errorReason().c_str());

    // ส่งและรับค่า Phosphorus
    Serial.printf("Set Phosphorus... %s\n", Firebase.setFloat(fbdo, "/Set/Phosphorus", phosphorus) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Get Phosphorus... %s\n", Firebase.getFloat(fbdo, "/Set/Phosphorus") ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
    int iVal_6 = 0;
    Serial.printf("Get Phosphorus ref... %s\n", Firebase.getFloat(fbdo, "//Set/Phosphorus", &iVal_6) ? String(iVal).c_str() : fbdo.errorReason().c_str());
    Serial.printf("Push Phosphorus... %s\n\n", Firebase.pushFloat(fbdo, "/Push/Phosphorus", phosphorus) ? "ok" : fbdo.errorReason().c_str());

    // ส่งและรับค่า Potassium
    Serial.printf("Set Potassium... %s\n", Firebase.setFloat(fbdo, "/Set/Potassium", potassium) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Get Potassium... %s\n", Firebase.getFloat(fbdo, "/Set/Potassium") ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
    int iVal_7 = 0;
    Serial.printf("Get Potassium ref... %s\n", Firebase.getFloat(fbdo, "//Set/Potassium", &iVal_7) ? String(iVal).c_str() : fbdo.errorReason().c_str());
    Serial.printf("Push Potassium... %s\n\n", Firebase.pushFloat(fbdo, "/Push/Potassium", potassium) ? "ok" : fbdo.errorReason().c_str());

    // ส่งและรับค่า Timestamp Thailand Timestamp GMT:+07.00
    Serial.printf("Set Timestamp... %s\n", Firebase.setFloat(fbdo, "/Set/Timestamp", epochTime) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Get Timestamp... %s\n", Firebase.getFloat(fbdo, "/Set/Timestamp") ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
    int iVal_8 = 0;
    Serial.printf("Get Timestamp ref... %s\n", Firebase.getFloat(fbdo, "//Set/Timestamp", &iVal_8) ? String(iVal).c_str() : fbdo.errorReason().c_str());
    Serial.printf("Push Timestamp... %s\n\n", Firebase.pushFloat(fbdo, "/Push/Timestamp", epochTime) ? "ok" : fbdo.errorReason().c_str());

    
    Serial.println();
    
    //For generic set/get functions.

    //For generic set, use Firebase.set(fbdo, <path>, <any variable or value>)

    //For generic get, use Firebase.get(fbdo, <path>).
    //And check its type with fbdo.dataType() or fbdo.dataTypeEnum() and
    //cast the value from it e.g. fbdo.to<int>(), fbdo.to<std::string>().

    //The function, fbdo.dataType() returns types String e.g. string, boolean,
    //int, float, double, json, array, blob, file and null.

    //The function, fbdo.dataTypeEnum() returns type enum (number) e.g. fb_esp_rtdb_data_type_null (1),
    //fb_esp_rtdb_data_type_integer, fb_esp_rtdb_data_type_float, fb_esp_rtdb_data_type_double,
    //fb_esp_rtdb_data_type_boolean, fb_esp_rtdb_data_type_string, fb_esp_rtdb_data_type_json,
    //fb_esp_rtdb_data_type_array, fb_esp_rtdb_data_type_blob, and fb_esp_rtdb_data_type_file (10)

    count++;
  }
}
