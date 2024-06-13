#define BLYNK_TEMPLATE_ID "TMPL6gLP9k2Jd"
#define BLYNK_TEMPLATE_NAME "New"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Replace with your Blynk authentication token, WiFi SSID, and password
char auth[] = "5TJM4Y3Qnd0VhMK6BS8PVC-KhlpA20ve";
char ssid[] = "ALTHAF TRAVELS";
char pass[] = "77F0B3f3";

// Fire detection system
#define LED1 D1
#define LED2 D8
#define Buzzer D3
#define Sensor D0
int pinValue = 0;

// Animal detection system
#define PIR_PIN D5
#define THEFT_LED_PIN D6
int pirState = LOW;
const int LED_DELAY_TIME = 1000;  // Delay time for the LED to stay on in milliseconds

// Weather monitoring system
#define DHTTYPE DHT22  // DHT 22 (AM2302)
#define DHTPIN D7      // GPIO4
DHT dht(DHTPIN, DHTTYPE);

// LCD display setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

BlynkTimer timer;

void setup() {
  Serial.begin(115200);

  // Fire detection system setup
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(Buzzer, OUTPUT);
  pinMode(Sensor, INPUT);

  // Theft detection system setup
  pinMode(PIR_PIN, INPUT);
  pinMode(THEFT_LED_PIN, OUTPUT);

  // Weather monitoring system setup
  dht.begin();

  // LCD display setup
  Wire.begin(D4, D2); // SDA on D4 and SCL on D8
  lcd.begin(16, 2); // Specify the number of columns and rows
  lcd.setBacklight(255); // Adjust backlight intensity if necessary

  // Initializing display
  lcd.setCursor(0, 0);
  lcd.print("  Initializing  ");
  for (int a = 5; a <= 10; a++) {
    lcd.setCursor(a, 1);
    lcd.print("+");
    delay(500);
  }
  lcd.clear();

  lcd.setCursor(0, 0); // Set the cursor to the first column, first row
  lcd.print("Fire & Animal");

  lcd.setCursor(0, 1); // Set the cursor to the first column, second row
  lcd.print("Detection System");

  Blynk.begin(auth, ssid, pass);

  // Delay to ensure connection
  delay(5000);

  timer.setInterval(1000L, fireNotification);
  timer.setInterval(2000L, weatherMonitoring);
}

BLYNK_WRITE(V0) {
  pinValue = param.asInt();
  updateDisplay();
}

void updateDisplay() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: " + String(temperature) + "C");

  lcd.setCursor(0, 1);
  lcd.print("Hum: " + String(humidity) + "%");

  lcd.setCursor(10, 1); // Bottom right position
  lcd.print("SS:");
  lcd.setCursor(13, 1);
  if (pinValue == 1) {
    lcd.print("ON");
  } else {
    lcd.print("OFF");
  }
}

void fireNotification() {
  int sensor = digitalRead(Sensor);
  if (pinValue == 1) {
    Serial.println("System is ON");
    if (sensor == 1) {
      digitalWrite(LED1, LOW);
      digitalWrite(LED2, HIGH);
      digitalWrite(Buzzer, LOW);
    } else if (sensor == 0) {
      digitalWrite(LED1, HIGH);
      digitalWrite(LED2, LOW);
      digitalWrite(Buzzer, HIGH);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("WARNING!");
      lcd.setCursor(0, 1);
      lcd.print("Fire Detected!");
      return;
    }
  } else if (pinValue == 0) {
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(Buzzer, LOW);
  }


  // Update weather info on LCD if no fire detected
  updateDisplay();
}

void weatherMonitoring() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, humidity);

  // Update weather info on LCD if no alerts are active
  if (pirState == LOW && digitalRead(Sensor) == 1) {
    updateDisplay();
  }
}

void loop() {
  Blynk.run();
  timer.run();

  if (pinValue == 1) {
    int pirStateCurrent = digitalRead(PIR_PIN);
    if (pirStateCurrent == HIGH) {
      if (pirState == LOW) {
        Serial.println("Motion detected!");
        Blynk.logEvent("alert_animal", "ALERT! Motion detected");
        digitalWrite(THEFT_LED_PIN, HIGH);
        pirState = HIGH;

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ALERT!");
        lcd.setCursor(0, 1);
        lcd.print("Animal Detected!");
        delay(500);
      }
    } else {
      if (pirState == HIGH) {
        Serial.println("Motion ended!");
        delay(LED_DELAY_TIME);
        digitalWrite(THEFT_LED_PIN, LOW);
        pirState = LOW;

        updateDisplay();
      }
    }
  }
}
