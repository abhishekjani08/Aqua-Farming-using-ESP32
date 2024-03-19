const int potPin = 36;
float ph;
float Value = 0.0;

void setup() {
  Serial.begin(115200);
  pinMode(18, OUTPUT); // LED pin
  pinMode(potPin, INPUT);
  delay(1000);
}

void loop() {
  Value = analogRead(potPin);
  Serial.print("Analog Value: ");
  Serial.print(Value);

  float voltage = Value * (3.3 / 4095.0);
  Serial.print(" | Voltage: ");
  Serial.print(voltage, 2);

  float slope = -7.50; // Adjust this value based on your calibration
  float intercept = 21.34; // Adjust this value based on your calibration
  ph = slope * voltage + intercept;

  Serial.print(" | pH: ");
  Serial.println(ph, 2);
  delay(1000);
  digitalWrite(18, HIGH); // turn the LED on
  delay(500);
  digitalWrite(18, LOW); // turn the LED off
  delay(500);
}