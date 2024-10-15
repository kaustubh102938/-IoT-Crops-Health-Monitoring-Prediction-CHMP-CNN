#include "DHT.h"
#include "Adafruit_Sensor.h"
#include "time.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN 3       // DHT sensor pin
#define DHTTYPE DHT11  // DHT sensor type
#define RELAY_PIN 22   // Relay control pin
#define trigPin 8      // Pin connected to the trigger pin (Trig) on the ultrasonic sensor
#define echoPin 5      // Pin connected to the echo pin (Echo) on the ultrasonic sensor
#define sensorPower 7  // Sensor pins
#define sensorPin 4

// Define LED pins
const int ledPins[] = { 10, 9, 8, 7, 6 };  // Connect LEDs to these digital pins

DHT dht(DHTPIN, DHTTYPE);

// Temperature thresholds
#define HIGH_TEMP_THRESHOLD 35    // Set your high temperature threshold in Celsius
#define MEDIUM_TEMP_THRESHOLD 33  // Set your medium temperature threshold in Celsius
#define LOW_TEMP_THRESHOLD 30     // Set your low temperature threshold in Celsius

int relayPin = 23;
int sensor_pin = A0;  // Soil Sensor input at Analog PIN A0

LiquidCrystal_I2C lcd(0x27, 16, 2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // Ensure the fan is initially turned off

  // Set ultrasonic sensor pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Set LED pins as OUTPUT
  for (int i = 0; i < sizeof(ledPins) / sizeof(ledPins[0]); i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  pinMode(relayPin, OUTPUT);
  pinMode(sensor_pin, INPUT);

  pinMode(sensorPower, OUTPUT);
  // Initially keep the sensor OFF
  digitalWrite(sensorPower, LOW);

  lcd.init();
  lcd.backlight();
}

void loop() {
  // Reading temperature from DHT sensor
  float temperatureC = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (!isnan(temperatureC) && !isnan(humidity)) {
    // Convert temperature to Fahrenheit
    float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;

    lcd.clear();
    lcd.setCursor(0, 0);
    Serial.print("Temperature: ");
    lcd.print("Temp: ");
    Serial.print(temperatureC);
    lcd.print(temperatureC);
    lcd.print(" C ");
    Serial.print("°C, ");
    lcd.print(temperatureF);
    lcd.print(" F ");
    Serial.println("°F");
    lcd.setCursor(0, 1);
    Serial.print("Humidity: ");
    lcd.print("Humidity: ");
    Serial.print(humidity);
    lcd.print(humidity);
    lcd.print("%");
    Serial.println("%");
    delay(3000);

    // Check temperature thresholds and control fan accordingly
    if (temperatureC >= HIGH_TEMP_THRESHOLD) {
      turnFanOn();
    } else if (temperatureC >= MEDIUM_TEMP_THRESHOLD) {
      // You can add additional actions for medium temperature if needed
      // For now, we'll turn the fan off
      turnFanOff();
    } else if (temperatureC <= LOW_TEMP_THRESHOLD) {
      turnFanOff();
    }
    // Trigger the ultrasonic sensor
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Read the echo duration and calculate distance
    long duration = pulseIn(echoPin, HIGH);
    int distance = duration * 0.034 / 2;  // Divide by 2 for one-way distance

    // Display water level using LEDs
    displayWaterLevel(distance);

    // Print distance to serial monitor
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Distance: ");
    lcd.print(" cm");
  }

  delay(5000);  // Delay for 5 seconds before the next reading

  int Soil_data = analogRead(sensor_pin);
  Serial.print("Soil_data:");
  Serial.print(Soil_data);
  Serial.print("\t | ");

  if (Soil_data > 950) {
    Serial.println("No moisture, Soil is dry");
    digitalWrite(relayPin, LOW);
  } else if (Soil_data >= 400 && Soil_data <= 950) {
    Serial.println("There is some moisture, Soil is medium");
    digitalWrite(relayPin, HIGH);
  } else if (Soil_data < 400) {
    Serial.println("Soil is wet");
    digitalWrite(relayPin, HIGH);
  }
  delay(3000);

  // Get the reading from the function below and print it
  int val = readSensor();
  Serial.print("Digital Output: ");
Serial.println(val);

  // Determine status of rain
  if (val) {
    lcd.clear();
    Serial.println("Status: Clear");
    lcd.setCursor(0, 0);
    lcd.print("Status: Clear    ");
  } else {
    Serial.println("Status: It's raining");
    lcd.setCursor(0, 0);
    lcd.print("Status: Raining  ");
  }

  delay(1000);	// Take a reading every second
}

void turnFanOn() {
  digitalWrite(RELAY_PIN, HIGH);  // Turn on the fan using the relay module
  Serial.println("Fan turned ON");
}

void turnFanOff() {
  digitalWrite(RELAY_PIN, LOW);  // Turn off the fan using the relay module
  Serial.println("Fan turned OFF");
}

void displayWaterLevel(int distance) {
  // Define water level thresholds in centimeters
  int waterLevels[] = { 25, 20, 15, 10, 5 };  // Change these values as needed

  for (int i = 0; i < sizeof(ledPins) / sizeof(ledPins[0]); i++) {
    if (distance >= waterLevels[i]) {
      digitalWrite(ledPins[i], LOW);
    } else {
      digitalWrite(ledPins[i], HIGH);
    }
  }
}

//  This function returns the sensor output
int readSensor() {
  digitalWrite(sensorPower, HIGH);   // Turn the sensor ON
  delay(10);                         // Allow power to settle
  int val = digitalRead(sensorPin);  // Read the sensor output
  digitalWrite(sensorPower, LOW);    // Turn the sensor OFF
  return val;                        // Return the value
}
