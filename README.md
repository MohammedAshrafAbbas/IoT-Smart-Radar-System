# 🚀 IoT Smart Radar & Traffic Enforcement System
### نظام الرادار الذكي المتكامل المدعوم بإنترنت الأشياء وكاميرا الهاتف

## 📋 نظرة عامة (Overview)
هذا المشروع هو حل هندسي متكامل لمراقبة حركة المرور وتطبيق قوانين السرعة آلياً. يعتمد النظام على متحكم **ESP32** لمعالجة البيانات لحظياً، حيث يقوم بحساب سرعة المركبات بدقة باستخدام مستشعرات الأشعة تحت الحمراء، ويوثق المخالفات بالصور عبر ربط لاسلكي مع كاميرا الهاتف الذكي، مع تخزين كامل للبيانات في السحابة.

---

## 🛠️ المميزات التقنية (Technical Features)

### 1. نظام القياس والتوثيق البصري (Speed & Vision)
* **Precision Calculation:** قياس الوقت الفاصل بين نقطتين بدقة ملي ثانية وتحويله لسرعة (km/h).
* **Smartphone Integration:** ربط الهاتف كـ **IP Camera** لالتقاط صور عالية الجودة فور رصد مخالفة.
* **Smart Flash System:** تفعيل فلاش الكاميرا تلقائياً في الإضاءة الضعيفة عبر مستشعر LDR.

### 2. الربط السحابي والتحكم عن بُعد (Cloud & IoT)
* **Firebase Integration:** قاعدة بيانات سحابية لحظية لتسجيل سجلات السرعة والمخالفات والبيانات البيئية.
* **Blynk Dashboard:** واجهة مستخدم تفاعلية لتعديل "حد السرعة" ومتابعة البيانات من أي مكان.

### 3. الواجهة التفاعلية والبيئة (UI & Environment)
* **Environmental Sensing:** مراقبة درجة الحرارة والرطوبة عبر مستشعر **DHT11**.
* **Real-time Feedback:** شاشة LCD لعرض البيانات، مع نظام تنبيه صوتي (Buzzer) وضوئي (LEDs).

---

## 📄 Project Description (English)

An advanced traffic management solution utilizing the **ESP32** to automate speed detection and violation logging. The system integrates hardware sensors with high-level cloud services and mobile imaging for a complete enforcement cycle.

**Key Technical Pillars:**
* **Automated Enforcement:** Accurately computes speed and triggers a smartphone camera via **HTTP protocol**.
* **IoT Ecosystem:** Full synchronization with **Firebase Realtime Database** and **Blynk IoT**.
* **Local Monitoring:** Real-time visualization on an I2C LCD and environmental tracking.

---

## 🏗️ Hardware Stack
* **Microcontroller:** ESP32.
* **Sensors:** (IR Obstacle, LDR Photocell, DHT11).
* **Actuators:** (Servo Motor, Buzzer, LEDs).
* **Display:** I2C LCD 16x2.
