#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 15       // Pin connected to the DHT sensor
#define DHTTYPE DHT11  // DHT sensor type (DHT11 or DHT22)

DHT dht(DHTPIN, DHTTYPE);

int RAIN_PIN = 23;

int MOTOR_PIN_1 = 25;
int MOTOR_PIN_2 = 26;

#include <WiFi.h>

const char* ssid = "The Simple Cafe";
const char* password = "simpleisthebest";


void setup(){
  pinMode(GPIO_NUM_2, OUTPUT);

  pinMode(RAIN_PIN,INPUT);

  pinMode(MOTOR_PIN_1,OUTPUT);
  pinMode(MOTOR_PIN_2,OUTPUT);

  Serial.begin(115200);

  dht.begin();

    WiFi.mode(WIFI_STA); //Optional
    WiFi.begin(ssid, password);
    Serial.println("\nConnecting");

    while(WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(500);
    }

}

void closeMaiche(){
  digitalWrite(MOTOR_PIN_1,HIGH);
  digitalWrite(MOTOR_PIN_2,LOW);
  delay(5000);
}

void openMaiche(){
  digitalWrite(MOTOR_PIN_1,LOW);
  digitalWrite(MOTOR_PIN_2,HIGH);
  delay(5000);
}

void stopMaiche(){
  digitalWrite(MOTOR_PIN_1,LOW);
  digitalWrite(MOTOR_PIN_2,LOW);
}

void loop(){
   Serial.println(WiFi.localIP());

  float temperature = dht.readTemperature(); 
  float humidity = dht.readHumidity();        

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C\tHumidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  int isRaining = digitalRead(23);

  if (isRaining == HIGH){
    Serial.println("Trời không mưa");
    closeMaiche();
    stopMaiche();
  }
  else{
    Serial.println("Trời đang mưa");
    openMaiche();
    stopMaiche();
  }

  delay(2000);
}

