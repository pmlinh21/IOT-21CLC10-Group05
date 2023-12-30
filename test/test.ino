#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include "PubSubClient.h"

#define DHT_PIN 15
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);

#define RAIN_PIN 23

#define MOTOR_PIN_1 25
#define MOTOR_PIN_2 26

float defaultTemperature = 35;  // Nhiệt độ mặc định tự động kéo mái che
float defaultHumidity = 80;     // Độ ẩm mặc định tự động báo khẩn cấp

float temperature;          // Đọc từ DHT11
float humidity;             // Đọc từ DHT11
int rain;                   // Đọc từ Rain Sensor - HIGH / LOW

// Giá trị mặc định ban đầu
bool isOpen = false;                // Mái che đang đóng
bool isRaining = false;             // Trời không mưa     (rain = HIGH)
bool isHot = false;                 // Trời không nóng    (temperature > 35)
bool isSafe = true;                 // Mạch điện an toàn (humidity < 80)
int mode = 1;                       // 1: Chế độ tự động "Bình thường", 2: Chế độ tự động "Phơi đồ", 3: Người dùng tự điều khiển

// Wifi Cafe
const char* ssid = "The Simple Cafe";
const char* password = "simpleisthebest";

// Wifi Lien
// const char* ssid = "YenLien";
// const char* password = "ngoquocanh";

// Wifi 3G
// const char* ssid = "21127341";
// const char* password = "hihihaha";

// Wifi Thư viện
// const char* ssid = "HCMUS Thu Vien";
// const char* password = "12345678";

const char*  mqttServer = "test.mosquitto.org";
int port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

void wifiConnect() {
  Serial.println("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");
}

void mqttReconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("21127341")) {
      Serial.println(" connected");
      client.subscribe("project/userOpenMaiche");

      // subcribe other topics here

    } else {
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  String stMessage;
  for (int i = 0; i < length; i ++) {
    stMessage += (char)message[i];
  }

  // Chế độ 3
  if (strcmp(topic, "project/userOpenMaiche")) {
    Serial.println(stMessage);
    // if (stMessage == "Mở") {
    //   Serial.println("Người dùng mở mái che");
    //   openMaiche();
    // }
    // else if (stMessage == "Đóng") {
    //   Serial.println("Người dùng đóng mái che");
    //   closeMaiche();
    // }
    // else {
    //   Serial.println("Sai topic hoặc sai message");
    // }
  }

  // Người dùng điều chỉnh nhiệt độ & độ ẩm mặc định
  // if(strcmp(topic, "project/changeDefaultValues")) {
  //   if (stMessage == ) 
  // }
}

void setup(){
  Serial.begin(115200);

  pinMode(RAIN_PIN, INPUT);

  pinMode(MOTOR_PIN_1, OUTPUT);
  pinMode(MOTOR_PIN_2, OUTPUT);
  digitalWrite(MOTOR_PIN_1,LOW);
  digitalWrite(MOTOR_PIN_2,LOW);

  dht.begin();
  
  pinMode(GPIO_NUM_2, OUTPUT);
  digitalWrite(GPIO_NUM_2, HIGH);
  wifiConnect();

  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqttServer, port);
  // client.setCallback(callback);
}

void closeMaiche(){
  digitalWrite(MOTOR_PIN_1,HIGH);
  delay(2000);
  digitalWrite(MOTOR_PIN_1,LOW);
}

void openMaiche(){
  digitalWrite(MOTOR_PIN_2,HIGH);
  delay(2000);
  digitalWrite(MOTOR_PIN_2,LOW);
}

void loop(){
  // mqtt status & reconnect
  if (!client.connected()) {
    mqttReconnect();
  }
  client.loop();

  // read input
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int rain = digitalRead(23);

  isRaining = (rain == LOW);

  // Message publish to MQTT
  char buffer[200];
  if (isnan(temperature) || isnan(humidity)) {
    isHot = false;
    isSafe = true;
    Serial.println("Failed to read from DHT sensor!");
    sprintf(buffer, "{\"defaultTemperature\":%.0f, \"defaultHumidity\":%.0f, \"temperature\":null, \"humidity\":null, \"isOpen\":%s, \"isRaining\":%s, \"isHot\":%s, \"isSafe\":%s, \"mode\":%d}", defaultTemperature, defaultHumidity, isOpen ? "true" : "false", isRaining ? "true" : "false", isHot ? "true" : "false", isSafe ? "true" : "false", mode);
  }
  else {
    isHot = (temperature > 35);
    isSafe = (humidity < 80);
    sprintf(buffer, "{\"defaultTemperature\":%.0f, \"defaultHumidity\":%.0f, \"temperature\":%.1f, \"humidity\":%.0f, \"isOpen\":%s, \"isRaining\":%s, \"isHot\":%s, \"isSafe\":%s, \"mode\":%d}", defaultTemperature, defaultHumidity, temperature, humidity, isOpen ? "true" : "false", isRaining ? "true" : "false", isHot ? "true" : "false", isSafe ? "true" : "false", mode);
  }
  Serial.println(buffer);
  client.publish("project/data", buffer);

  if (mode == 1) {
    // chế độ "Bình thường"
  }
  else if (mode == 2) {
    // chế độ "Phơi đồ"
  }
  else if (mode == 3) {
    // chế độ "Tự điều khiển"
    // do nothing, only receive msg from mqtt
  }

  delay(100);
}