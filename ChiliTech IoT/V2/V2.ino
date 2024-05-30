// Mendefinisikan ID dan nama template Blynk serta token otentikasi
#define BLYNK_TEMPLATE_ID "TMPL6JyLFv16L"
#define BLYNK_TEMPLATE_NAME "Capstone Project Indobot Sib 6 Fix"
#define BLYNK_AUTH_TOKEN "cAGHmjgYGXcCYT9VL29RjgBjvBtSX3ge"

// Memuat library untuk ESP8266 dan Blynk
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// Memuat library untuk LCD I2C
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, D3, D4);

// Mendefinisikan pin waterFlowSensor dan katup
#define waterFlowSensor D7
int selenoidValve = D8;

#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

#define FIREBASE_HOST "chilitech-ytcraft-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "AIzaSyCdsOVWqG81uosME5krgcPciXFklREMx1w"

#define WIFI_SSID     "WIFI_Premium"
#define WIFI_PASSWORD "senyumdulu"

// Mendeklarasikan objek data dari FirebaseESP8266
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

const int sensorSoilMosturePin = A0;

int sensor1 = D1;
int sensor2 = D2;
int sensor3 = D5;
int sensor4 = D6;

char authBlynk[] = "cAGHmjgYGXcCYT9VL29RjgBjvBtSX3ge";

String LEVEL = "00";

// Mendefinisikan variabel dan konstanta untuk pengukuran aliran
long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

const int AirValue = 2650;
const int WaterValue = 980;
int soilMoistureValue = 0;
int soilmoist = 0;

// Fungsi interupsi untuk menghitung pulse
void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

void setup() {

  Serial.begin(9600);

  // Memulai koneksi ke server Blynk
  Blynk.begin(authBlynk, WIFI_SSID, WIFI_PASSWORD);

  // Mengatur pin waterFlowSensor dan katup
  pinMode(waterFlowSensor, INPUT_PULLUP);
  pinMode(selenoidValve, OUTPUT);

  // Menginisialisasi variabel pengukuran aliran
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;

  // Melampirkan fungsi interupsi ke pin waterFlowSensor
  attachInterrupt(digitalPinToInterrupt(waterFlowSensor), pulseCounter, FALLING);

  pinMode(sensor1, INPUT_PULLUP);
  pinMode(sensor2, INPUT_PULLUP);
  pinMode(sensor3, INPUT_PULLUP);
  pinMode(sensor4, INPUT_PULLUP);
  // pinMode(LED_BUILTIN, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP Address is : ");
  Serial.println(WiFi.localIP());
  
  // Konfigurasi Firebase
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;

  Firebase.reconnectWiFi(true);
  Firebase.begin(&config, &auth);

  Serial.println();
  delay(1000);
}

void loop() {
  // Instalasi LCD
  lcd.begin();
  lcd.backlight();
  
  // Mendapatkan waktu saat ini
  currentMillis = millis();
  
  // Memeriksa apakah interval waktu sudah tercapai
  if (currentMillis - previousMillis > interval) {
    
    // Menyimpan jumlah pulse per detik
    pulse1Sec = pulseCount;
    
    // Reset jumlah pulse
    pulseCount = 0;
    
    // Menghitung kecepatan aliran
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    
    // Memperbarui waktu sebelumnya
    previousMillis = millis();
    
    // Menghitung aliran dalam mililiter
    flowMilliLitres = (flowRate / 60) * 1000;
    
    // Menambahkan aliran dalam mililiter ke total
    totalMilliLitres += flowMilliLitres;

    // Mencetak kecepatan aliran dan total aliran
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));
    Serial.print("L/min");
    Serial.print("\t");
    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(totalMilliLitres / 1000);
    Serial.println("L");

    // Mengirim data ke aplikasi Blynk
    Blynk.virtualWrite(V0, totalMilliLitres);
    Blynk.virtualWrite(V2, flowRate);
    read_SoilMoist();
    Blynk.virtualWrite(V4, soilmoist);
    
    // Menjalankan Blynk
    Blynk.run();
  }

  // digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on

  if (digitalRead(sensor4) == LOW) {
    LEVEL = "100";
  } else if (digitalRead(sensor3) == LOW) {
    LEVEL = "75";
  } else if (digitalRead(sensor2) == LOW) {
    LEVEL = "50";
  } else if (digitalRead(sensor1) == LOW) {
    LEVEL = "25";
  } else if (digitalRead(sensor1) == HIGH && digitalRead(sensor2) == HIGH && digitalRead(sensor3) == HIGH && digitalRead(sensor4) == HIGH) {
    LEVEL = "00";
  }

  if (Firebase.setString(firebaseData, "/level", LEVEL)) {
    Serial.print("level value ");
    Serial.print(LEVEL);
    Serial.println(" Uploaded Successfully");
  } else {
    Serial.println(firebaseData.errorReason());
  }

  Serial.println();

  delay(100);
  // digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off
  delay(3000);
}

void read_SoilMoist() {
  soilMoistureValue = sensorSoilMosturePin;
  soilmoist= (100 - ((soilMoistureValue / 1023.00) * 100));
  
  // if(soilmoist >= 100) {
  //   soilmoist=100;
  // } else if(soilmoist <=0) {
  //   soilmoist=0;
  // }

  Serial.print("Soil Humid = ");
  Serial.print(soilmoist);
  Serial.println("%");
  
  lcd.setCursor(0,0);
  lcd.print("Soil Humid =");
  lcd.setCursor(13,0);
  lcd.print(soilmoist);   
  lcd.print("% ");
}

// Fungsi ini akan dipanggil setiap kali tombol widget pada aplikasi Blynk menulis nilai ke Pin Virtual V1
BLYNK_WRITE(V1) {
  int pinValue = param.asInt();         // Menetapkan nilai masukan dari pin V3 ke sebuah variabel
  if (pinValue == 1) {
    digitalWrite(selenoidValve, HIGH);  // Menghidupkan katup
  } else {
    digitalWrite(selenoidValve, LOW);   // Mematikan katup
  }
}

BLYNK_WRITE(V3) {
  ESP.restart();
}