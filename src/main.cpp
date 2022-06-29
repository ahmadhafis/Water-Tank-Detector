#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Home Wifi
const char *ssid = "JTI-POLINEMA";
const char *password = "jtifast!";

// Broker
const char *mqtt_server = "broker.hivemq.com";

// Define Subscribed Topics
WiFiClient espClient;
PubSubClient client(espClient);

long now = millis();
long lastMeasure = 0;
String macAddr = "";

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
  macAddr = WiFi.macAddress();
  Serial.println(macAddr);
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(macAddr.c_str()))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void connnection()
{
  if (!client.connected())
  {
    reconnect();
  }
  if (!client.loop())
  {
    client.connect(macAddr.c_str());
  }
  now = millis();
}

#define triggerPin D6
#define echoPin D7
int relaypin = D5;

void config()
{
  connnection();
  if (now - lastMeasure > 5000)
  {
    lastMeasure = now;
    int duration, jarak;
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    jarak = duration * 0.034 / 2;
    Serial.print(jarak);
    Serial.println(" cm");
    delay(2000);

    // Define Percentage
    int percent;
    if (jarak <= 3)
    {
      percent = 100;
    }
    else if (jarak >= 10)
    {
      percent = 0;
    }
    else
    {
      percent = (10 - jarak) * 10;
    }

    lcd.setCursor(0, 0);
    lcd.print("Water Level:");
    lcd.print(percent);
    lcd.print("  ");

    static char percentTemp[7];
    dtostrf(percent, 4, 2, percentTemp);
    Serial.println(percentTemp);
    client.publish("1941720172/percentage", percentTemp);

    static char sensorDistance[7];
    dtostrf(jarak, 4, 2, sensorDistance);
    Serial.println(sensorDistance);
    client.publish("1941720172/sensordistance", sensorDistance);

    // Relay
    if (jarak >= 10)
    {
      digitalWrite(relaypin, HIGH);
      lcd.setCursor(0, 1);
      lcd.print("Motor is ON ");
      client.publish("1941720172/motorstatus", "ON");
    }
    else if (jarak <= 3)
    {
      digitalWrite(relaypin, LOW);
      lcd.setCursor(0, 1);
      lcd.print("Motor is OFF");
      client.publish("1941720172/motorstatus", "OFF");
    }
  }
}

void setup()
{
  Serial.begin(115200);
  lcd.init(); // initialize the lcd
  lcd.backlight();
  lcd.clear();
  lcd.home();
  pinMode(relaypin, OUTPUT);
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.println("Mqtt Node-RED");
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}


void loop()
{
  config();
  lcd.home();
}