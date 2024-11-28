#include <DHT.h>
#include <DHT_U.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


LiquidCrystal_I2C lcd(0x27, 16, 2); 

const char* ssid = "Your_ssid";
const char* password = "Your_Password";

//Only for project use DB :xD
#define API_KEY "Your_FirebaseRTb_apiKey"
#define DATABASE_URL "Your_FirebaseRTb_url"


#define SOIL_SENSOR_PIN A0
#define DHTPIN D5

#define RELAY_PIN1 D1
#define RELAY_PIN2 D6
#define RELAY_PIN3 D7
#define RELAY_PIN4 D8


#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

bool r1 = false;
bool r2 = false;
bool r3 = false;
bool r4 = false;

int soilMoisture = 0;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

void setup() {
  Serial.begin(115200);

  dht.begin();


  pinMode(RELAY_PIN1, OUTPUT);
  pinMode(RELAY_PIN2, OUTPUT);
  pinMode(RELAY_PIN3, OUTPUT);
  pinMode(RELAY_PIN4, OUTPUT);

//Ye dono pin kv kisi ko assign nhi karna h lcd ke alawa
  Wire.begin(D2, D4); 

 
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Moisture Monitor");


  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");



  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

 
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Sign-up successful");
    signupOK = true;
  } else {
    Serial.printf("Sign-up failed: %s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);


}

void loop() {

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();


  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C  ");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  }


  int ReadsoilMoisture = analogRead(SOIL_SENSOR_PIN);
  soilMoisture = map(ReadsoilMoisture, 0, 1023, 100, 0);

//Analog value moisture sensor ka max 1023 hoga lekin agar without capacitor se filter kiye signal read krna ho to body ya kisi tarah ka static charge se value upar ja sakti h jisse percentage v 100+ jayega isliye ye krna jaruri h

  if (soilMoisture > 100) {
    soilMoisture = 100;
  } else if (soilMoisture < 0) {
    soilMoisture = 0;
  }

 
  Serial.print("Soil Moisture (Analog Value): ");
  Serial.println(ReadsoilMoisture);

  Serial.print("Soil Moisture (Percentage Value): ");
  Serial.println(soilMoisture);

  if (Firebase.ready() && signupOK) {
    
    if (!Firebase.RTDB.setInt(&fbdo, "/IistBiharData/SoilMoisture_DHT_4Relay/humidity", humidity)) {
      Serial.println("Failed to update humidity to Firebase");
    }

   
    if (!Firebase.RTDB.setInt(&fbdo, "/IistBiharData/SoilMoisture_DHT_4Relay/temp", temperature)) {
      Serial.println("Failed to update temperature to Firebase");
    }
    
    if (!Firebase.RTDB.setInt(&fbdo, "/IistBiharData/SoilMoisture_DHT_4Relay/MoistureValue", soilMoisture)) {
      Serial.println("Failed to update soilMoisture to Firebase");
    }
  }

  if (soilMoisture < 30) {
    r1 = true;
    digitalWrite(RELAY_PIN1, LOW);
    Serial.println("Soil is Dry - Pump ON");


  } else if (soilMoisture < 70) {

    Serial.println("Soil is moist");

  } else {
    r1 = false;
    
    
    digitalWrite(RELAY_PIN1, HIGH);
    Serial.println("Soil is Wet - Pump OFF");

  }


  if (Firebase.ready() && signupOK) {
    if (Firebase.RTDB.getInt(&fbdo, "/IistBiharData/SoilMoisture_DHT_4Relay/relay2")) {
      int status = fbdo.intData();
      if (status == 1) {
        digitalWrite(RELAY_PIN2, LOW);
        r2 = true;
        Serial.println("Switch 2 on");
        

      } else {
        digitalWrite(RELAY_PIN2, HIGH);
         r2 = false;
        Serial.println("Switch 2 OFF");

      }
    }
    if (Firebase.RTDB.getInt(&fbdo, "/IistBiharData/SoilMoisture_DHT_4Relay/relay3")) {
      int status = fbdo.intData();
      if (status == 1) {
        digitalWrite(RELAY_PIN3, LOW);
        r3 = true;
        Serial.println("Switch 3 on");

      } else {
        digitalWrite(RELAY_PIN3, HIGH);
        r3 = false;
        Serial.println("Switch 3 OFF");

      }
    }
    if (Firebase.RTDB.getInt(&fbdo, "/IistBiharData/SoilMoisture_DHT_4Relay/relay4")) {
      int status = fbdo.intData();
      if (status == 1) {
        digitalWrite(RELAY_PIN4, LOW);
        r4 = true;
        Serial.println("Switch 4 on");

      } else {
        digitalWrite(RELAY_PIN4, HIGH);
        r4 = false;
        Serial.println("Switch 4 OFF");

      }
    }




  }



 
  lcd.clear();

 
  lcd.setCursor(0, 0);

  lcd.print("T:");
  lcd.print(temperature);
  lcd.print("C H:");
  lcd.print(humidity);
  lcd.print("g/m³");
  lcd.setCursor(0, 1);
  lcd.print("M:");
  lcd.print(soilMoisture);
  lcd.print("% MS.ON:");
  if (r1) {
    lcd.print("1");
  } 
    if (r2) {
    lcd.print("2");
  }
  if (r3) {
    lcd.print("3");
  }
    if (r4) {
    lcd.print("4");
  }

}
