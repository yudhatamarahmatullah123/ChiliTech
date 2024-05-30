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
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Mendefinisikan pin waterFlowSensor dan katup
#define waterFlowSensor D4
int selenoidValve = D3;

// Menyimpan informasi jaringan WiFi dan auth token firmware Blynk
// char ssid[] = "WIFI_Premium";
// char pass[] = "senyumdulu";
//char ssid[] = "Ktr";
//char pass[] = "ktr12345678";
char ssid[] = "yt";
char pass[] = "bismillah";
char auth[] = "cAGHmjgYGXcCYT9VL29RjgBjvBtSX3ge";

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
  // Memulai komunikasi serial
  Serial.begin(115200);

  // Memulai koneksi ke server Blynk
  Blynk.begin(auth, ssid, pass);

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
}

void read_SoilMoist() {
  soilMoistureValue = analogRead(D2);
  soilmoist= (100 - ((soilMoistureValue / 1023.00) * 100));
  if(soilmoist >= 100)
    {
      soilmoist=100;
    }
  else if(soilmoist <=0)
    {
      soilmoist=0;
    }
  
  lcd.setCursor(0,0);
  lcd.print("Soil Humid =");
  lcd.setCursor(13,0);
  lcd.print(soilmoist);   
  lcd.print("% ");

  if(soilmoist >= 80) {
    digitalWrite(selenoidValve, HIGH);  // Menghidupkan katup
  } else if(soilmoist <= 60){
    digitalWrite(selenoidValve, LOW);  // Mematikan katup
  } else {
    digitalWrite(selenoidValve, LOW);  // Menghidupkan katup
  }
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
