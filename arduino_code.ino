#include <PubSubClient.h>
#include <Ethernet.h>
#include <SimpleDHT.h>

byte mac[] = { 0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
byte server[] = { 192, 168, 178, 74 };
EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

int LDR;
int moist;

uint16_t ldrVal;
uint16_t moistVal;

String moisture;

float temp;
float humidity;
float lux=0.00,ADC_val=0.0048828125;

uint8_t ldrAnalogPin = A0;
uint8_t moistureAnalogPin = A1;
int moistureDigitalPinOutput = 2;
int moistureDigitalPinInput = 3;
int temperatureDigitalPin = 4;

SimpleDHT11 dht11(temperatureDigitalPin);

void setup() {
  Serial.begin(9600);

  Ethernet.begin(mac);
  delay(1500);
  mqttClient.setServer(server, 1883);

  if (connectMqtt()) {
    Serial.println("connected to MQTT");
  } else {
    Serial.println(mqttClient.state());
  }
  
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(moistureDigitalPinInput, INPUT);
  pinMode(moistureDigitalPinOutput, OUTPUT);
  digitalWrite(moistureDigitalPinOutput, LOW);
}

void loop() {
if (!mqttClient.connected()) {
  if (connectMqtt()) {
    Serial.println("connected to MQTT");
  } else {
    Serial.println(mqttClient.state());
  }
}
  LDR = analogRead(A0);
  lux = (250/(ADC_val*LDR))-50;
  ldrVal = map(LDR, 0, 1023, 100, 0);
  Serial.print("Lichtintensität: ");
  Serial.print(lux);
  Serial.println(" Lux\n");

  digitalWrite(2, HIGH);
  delay(10);
  moist = analogRead(A1);
  digitalWrite(moistureDigitalPinOutput, LOW);
  moistVal = map(moist, 0, 1023, 100, 0);
  Serial.print("Rel. Bodenfeuchtigkeit: ");
  Serial.print(moistVal);
  Serial.println("%\n");

  temp = 0;
  humidity = 0;

  int err = SimpleDHTErrSuccess;

  if ((err = dht11.read2(&temp, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print ("Read DHT11 failed, err=");
    Serial.println(err);
    delay(2000);
    return;
  }

  Serial.print("Temperatur: ");
  Serial.print((float)temp);
  Serial.println(" *C");

  Serial.print("Rel. Luftfeuchtigkeit: ");
  Serial.print((float)humidity);
  Serial.println(" RH%\n\n\n");

  publishValue("temp", temp);
  publishValue("lumi", lux);
  publishValue("humi", humidity);
  publishValue("moist", float(moistVal));
  
  
  delay(10000);
}

boolean connectMqtt() {
  return mqttClient.connect("arduino1", "arduino", "hallo123");
}

boolean publishValue(char topic[], float value) {
  char convertedVal[8];
  dtostrf(value, 6, 2, convertedVal);
  if (mqttClient.publish(topic, convertedVal)) {
    Serial.println("Publish successful!");
  } else {
    Serial.println("Publish failed!");
  }
}
