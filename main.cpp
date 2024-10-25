#include <Arduino.h>
#include <TFT_eSPI.h>
#include <DHTesp.h>

#define             RELAY 17

#define             DHTPIN 18
DHTesp              dht;
int                 interval = 2000;
unsigned long       lastDHTReadMillis = 0;
float               humidity = 0;
float               temperature = 0;
char                dht_buf[50];

TFT_eSPI            tft = TFT_eSPI();

const int           pulseA = 44;
const int           pulseB = 43;
volatile int        lastEncoded = 0;
volatile long       encoderValue = 0;
char                rotary_buf[20];

void readDHT22() {
    unsigned long currentMillis = millis();

    if(millis() > lastDHTReadMillis + interval) {
        lastDHTReadMillis = currentMillis;

        humidity = dht.getHumidity();
        temperature = dht.getTemperature();
    }
}

IRAM_ATTR void handleRotary() {
    // Never put any long instruction
    int MSB = digitalRead(pulseA); //MSB = most significant bit
    int LSB = digitalRead(pulseB); //LSB = least significant bit

    int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
    int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value
    if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
    if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;
    lastEncoded = encoded; //store this value for next time
    encoderValue = encoderValue > 255 ? 255 : (encoderValue < 0 ? 0 : encoderValue);
}

void setup() {
    Serial.begin(115200);
    dht.setup(DHTPIN, DHTesp::DHT22);   // Connect DHT Sensr to GPIO 18
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("Thermostat", 80, 20, 4);
    tft.drawString("Temp : ", 30, 60, 2);
    tft.drawString("Set  : ", 30, 90, 2);

    pinMode(pulseA, INPUT_PULLUP);
    pinMode(pulseB, INPUT_PULLUP);
    pinMode(RELAY, OUTPUT);
    attachInterrupt(pulseA, handleRotary, CHANGE);
    attachInterrupt(pulseB, handleRotary, CHANGE);
}

void loop() {
    readDHT22();
    int rotary = map(encoderValue, 0, 255, 0, 60);
    sprintf(rotary_buf, "%d", rotary);
    sprintf(dht_buf, "%.1f", temperature);
    tft.fillRect(90, 60, 90, 50, TFT_BLACK);
    tft.drawString(dht_buf, 90, 60, 2);
    tft.drawString(rotary_buf, 90, 90, 2);
    if (rotary > temperature) {
        digitalWrite(RELAY, HIGH);
    } else {
        digitalWrite(RELAY, LOW);
    }
    delay(500);
}
