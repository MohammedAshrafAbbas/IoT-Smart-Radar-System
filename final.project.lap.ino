// ================================================================ 
//  RADAR SYSTEM - ESP32 (IOT CONTROLLED + DUAL CAMERA CAPTURE) 
// ================================================================  

#define BLYNK_TEMPLATE_ID "TMPL2-5d5neSj"
#define BLYNK_TEMPLATE_NAME "radar"
#define BLYNK_AUTH_TOKEN "qArorqiOyFFy4JDWbCvhJjXoAQ3K0sn5"  

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#define FIREBASE_RTDB_ONLY
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"  

// ── WiFi & Servers ─────────────────────────────────────────────
const char* ssid = "WE_AB0A77";
const char* pass = "jcw09394";
String serverIP  = "http://192.168.1.3:3000/capture";  

// ── Firebase ───────────────────────────────────────────────────
#define FIREBASE_HOST "radarproject-dd214-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "ZOEhr1MqxHA3umgku4lfOjJGJdP780BmNAP0xc2T"  

// ── Pins ────────────────────────────────────────────────────────
const int IR1_PIN       = 34;   
const int IR2_PIN       = 35;   
const int LDR_PIN       = 32;
const int LED_RED_PIN   = 25;
const int LED_GREEN_PIN = 26;
const int BUZZER_PIN    = 27;
const int SERVO_PIN     = 13;
const int DHT_PIN       = 4;  

// ── Dynamic Variables ───────────────────────────────────────────
const float SENSOR_DISTANCE_M = 0.18;   
float SPEED_LIMIT_KMH         = 80.0; 
const int   SERVO_LEFT        = 135;    
const int   SERVO_CENTER      = 90;     
const int   IR_TIMEOUT_MS     = 5000;  
const int   SERVO_MOVE_MS     = 150;   
const int   LDR_DARK_THRESHOLD = 500;   

// ── Objects ─────────────────────────────────────────────────────
DHT                dht(DHT_PIN, DHT11);
Servo              myServo;
LiquidCrystal_I2C  lcd(0x27, 16, 2);
FirebaseData       fbdo;
FirebaseAuth       auth;
FirebaseConfig     config;
BlynkTimer         timer;
WidgetLED          blynkRed(V4);
WidgetLED          blynkGreen(V5);  

// ── State ────────────────────────────────────────────────────────
volatile bool isProcessing  = false;
float         lastSpeed     = 0;
float         lastTemp      = 0; 
float         lastHum       = 0;  

// ================================================================ 
//  BLYNK IOT CONTROL 
// ================================================================  

BLYNK_WRITE(V6) { 
  SPEED_LIMIT_KMH = param.asFloat();      
  Blynk.virtualWrite(V7, SPEED_LIMIT_KMH);   
  Serial.print("[Blynk] : Max Speed Updated to: ");
  Serial.println(SPEED_LIMIT_KMH);  
  
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.print("Max Speed:"); 
  lcd.print(SPEED_LIMIT_KMH, 1); 
  lcd.print("km");
}  

BLYNK_CONNECTED() { 
  Blynk.syncVirtual(V6); 
}  

// ================================================================ 
//  HELPERS & ALERTS  
// ================================================================ 
bool shouldFlash() { 
  return analogRead(LDR_PIN) < LDR_DARK_THRESHOLD;
}  

void requestCapture(float speed, bool withFlash) { 
  if (WiFi.status() != WL_CONNECTED) return; 
  HTTPClient http; 
  http.setTimeout(1000); 
  String url = serverIP + "?speed=" + String(speed, 2) + 
               "&limit=" + String(SPEED_LIMIT_KMH, 1) + 
               "&flash=" + (withFlash ? "on" : "off");      
  http.begin(url); 
  int code = http.GET(); 
  http.end();
}  

void updateLCD(float speed, float t, float h) { 
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.print("Max Speed:"); 
  lcd.print(SPEED_LIMIT_KMH, 1); 
  lcd.print("km"); 
  lcd.setCursor(0, 1); 
  lcd.print("S:"); 
  lcd.print(speed, 2); 
  lcd.print(" T:"); lcd.print((int)t); 
  lcd.print(" H:"); lcd.print((int)h); 
}  

void sendEnvironmentData() { 
  float h = dht.readHumidity(); 
  float t = dht.readTemperature(); 
  if (isnan(h) || isnan(t)) return; 
  lastTemp = t; lastHum = h; 
  if (Blynk.connected()) { Blynk.virtualWrite(V0, t); Blynk.virtualWrite(V1, h); } 
  if (Firebase.ready()) { 
    Firebase.RTDB.setFloat(&fbdo, "/Environment/Temp", t); 
    Firebase.RTDB.setFloat(&fbdo, "/Environment/Hum",  h); 
  }
}  

void violationAlert() { 
  if (Blynk.connected()) blynkRed.on(); 
  for (int i = 0; i < 3; i++) { 
    digitalWrite(LED_RED_PIN, HIGH); 
    digitalWrite(BUZZER_PIN, HIGH); 
    delay(150); 
    digitalWrite(LED_RED_PIN, LOW); 
    digitalWrite(BUZZER_PIN, LOW); 
    delay(150); 
  } 
  if (Blynk.connected()) blynkRed.off();
}  

void normalAlert() { 
  if (Blynk.connected()) blynkGreen.on(); 
  digitalWrite(LED_GREEN_PIN, HIGH); 
  delay(2000); 
  digitalWrite(LED_GREEN_PIN, LOW); 
  if (Blynk.connected()) blynkGreen.off();
}  

void uploadResult(float speed, bool violation) { 
  if (!Firebase.ready()) return; 
  Firebase.RTDB.setFloat(&fbdo,  "/Radar/LastSpeed", speed); 
  Firebase.RTDB.setString(&fbdo, "/Radar/Status", violation ? "Violation" : "Normal"); 
  if (Blynk.connected()) { Blynk.virtualWrite(V3, speed); }
}  

// ================================================================ 
//  CORE RADAR LOGIC 
// ================================================================ 
void handleDetection() { 
  Serial.println("════════════════════════════════"); 
  Serial.println("[Phase 1] IR1 Triggered"); 
  
  bool flash = shouldFlash(); 
  Serial.println("[Camera] Capture 1: Early detection..."); 
  requestCapture(0, flash); 
  
  myServo.write(SERVO_CENTER); 
  Serial.println("[Servo] Moving to center..."); 
  unsigned long servoMoveStart = millis(); 
  while (millis() - servoMoveStart < SERVO_MOVE_MS) { 
    if (WiFi.status() == WL_CONNECTED) Blynk.run(); 
    timer.run(); 
    yield(); 
  }  

  unsigned long clearStart = millis(); 
  while (digitalRead(IR1_PIN) == LOW && millis() - clearStart < 2000) { yield(); }  

  Serial.println("[Phase 2] Waiting for IR1 (Front Edge)..."); 
  unsigned long phase2Start = millis(); 
  bool ir1FrontTriggered = false; 
  unsigned long t1 = 0;  

  while (millis() - phase2Start < IR_TIMEOUT_MS) { 
    if (digitalRead(IR1_PIN) == LOW) { 
      t1 = millis(); 
      ir1FrontTriggered = true; 
      break; 
    } 
    yield(); 
  }  

  if (!ir1FrontTriggered) { 
    myServo.write(SERVO_LEFT); 
    isProcessing = false; 
    return; 
  }  

  Serial.println("[Phase 3] Waiting for IR2..."); 
  unsigned long phase3Start = millis(); 
  bool ir2Triggered = false; 
  unsigned long t2 = 0;  

  while (millis() - phase3Start < IR_TIMEOUT_MS) { 
    if (digitalRead(IR2_PIN) == LOW) { 
      t2 = millis(); 
      ir2Triggered = true; 
      break; 
    } 
    yield(); 
  }  

  float speed = 0; 
  if (ir2Triggered && t2 > t1) { 
    float timeSec = (t2 - t1) / 1000.0; 
    speed = (SENSOR_DISTANCE_M / timeSec) * 3.6; 
    Serial.printf("[Result] Speed: %.2f km/h\n", speed);  

    Serial.println("[Camera] Capture 2: Final Evidence..."); 
    requestCapture(speed, flash); 
  }  

  updateLCD(speed, lastTemp, lastHum); 
  bool violation = (speed > SPEED_LIMIT_KMH); 
  uploadResult(speed, violation);  

  if (violation) violationAlert(); else normalAlert();  

  myServo.write(SERVO_LEFT); 
  delay(SERVO_MOVE_MS); 
  isProcessing = false; 
  Serial.println("════════════════════════════════\n");
}  

// ================================================================ 
//  SETUP & LOOP 
// ================================================================ 
void setup() { 
  Serial.begin(115200); 
  lcd.begin(); // تم تغيير begin إلى init لضمان التوافق مع أغلب مكتبات I2C
  lcd.backlight(); 
  lcd.print("IoT Radar Ready");  

  pinMode(IR1_PIN, INPUT);
  pinMode(IR2_PIN, INPUT); 
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);  

  myServo.attach(SERVO_PIN); 
  myServo.write(SERVO_LEFT);  

  dht.begin(); 
  WiFi.begin(ssid, pass); 
  unsigned long wifiStart = millis(); 
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < 10000) { delay(500); Serial.print("."); }  

  if (WiFi.status() == WL_CONNECTED) { 
    Blynk.config(BLYNK_AUTH_TOKEN); 
    config.database_url = FIREBASE_HOST; 
    config.signer.tokens.legacy_token = FIREBASE_AUTH; 
    Firebase.begin(&config, &auth); 
  }  

  timer.setInterval(5000L, sendEnvironmentData); 
  updateLCD(0, 0, 0);
}  

void loop() { 
  if (WiFi.status() == WL_CONNECTED) Blynk.run(); 
  timer.run();  

  if (isProcessing) return;  

  if (digitalRead(IR1_PIN) == LOW) { 
    delay(10); 
    if (digitalRead(IR1_PIN) == LOW) { 
      isProcessing = true; 
      handleDetection(); 
    } 
  }
}