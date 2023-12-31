#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include "PubSubClient.h"
#include <FirebaseESP32.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

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
bool warning =false;

const char* host = "maker.ifttt.com";
const char* requestAwningOpen ="/trigger/awning_open/with/key/O1GD7w-u_n1I9S17Dn4u_"; //mở mái che
const char* requestAwningClose ="/trigger/awning_close/with/key/O1GD7w-u_n1I9S17Dn4u_";//đóng mái che
const char* requestHumidityWarning ="/trigger/humidity_warning/with/key/O1GD7w-u_n1I9S17Dn4u_";//cảnh báo độ ẩm cao

const int iftttPort = 80;

#define DATABASE_URL "https://iot-group-5-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define API_KEY "AIzaSyCMhw66Wjzlnw8JKPE_wHaunJ2kp-tFrcA"

#define USER_EMAIL "iot.nhom.5@gmail.com"
#define USER_PASSWORD "iot.group.5"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Wifi Cafe
// const char* ssid = "The Simple Cafe";
// const char* password = "simpleisthebest";

// Wifi Lien
// const char* ssid = "YenLien";
// const char* password = "ngoquocanh";

// Wifi 3G
// const char* ssid = "21127341";
// const char* password = "hihihaha";

// Wifi Thư viện
// const char* ssid = "HCMUS Thu Vien";
// const char* password = "12345678";

const char* ssid = "Khanh Ngoc";
const char* password = "Giaquy@@2015pcduc";

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
      // client.subscribe("project/userChangeAutoMode");
      // client.subscribe("project/userOpenAwning");
      // client.subscribe("project/userChangeMode");
      // client.subscribe("project/userSetDefaultTemp");
      // client.subscribe("project/userSetDefaultHumid");
      // client.subscribe("project/userSetDefaultValues");
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
  Serial.println(topic);
  // if (strcmp(topic, "project/userChangeAutoMode") == 0) {
  //   if (stMessage == "1") {
  //     Serial.println("Người dùng chuyển chế độ Bình thường");
  //     autoMode = 1;
  //   }
  //   else if (stMessage == "2") {
  //     Serial.println("Người dùng chuyển chế độ Phơi đồ");
  //     autoMode = 2;
  //   }
  //   else {
  //     Serial.println("Lỗi chuyển chế độ tự động");
  //   }
  //   Serial.println(stMessage);
  // } else if (strcmp(topic, "project/userOpenAwning") == 0) {
  //   if (stMessage == "true") {
  //     Serial.println("Người dùng mở mái che");
  //     openMaiche();
  //     isOpen = true;
  //   }
  //   else if (stMessage == "false") {
  //     Serial.println("Người dùng đóng mái che");
  //     closeMaiche();
  //     isOpen = false;
  //   }
  //   else {
  //     Serial.println("Lỗi đóng mở mái che");
  //   }
  //   Serial.println(stMessage);
  // } else if (strcmp(topic, "project/userChangeMode") == 0) {
  //   if (stMessage == "auto") {
  //     Serial.println("Người dùng chuyển chế độ Tự động");
  //     isAuto = true;
  //   }
  //   else if (stMessage == "manual") {
  //     Serial.println("Người dùng chuyển chế độ Thủ công");
  //     isAuto = false;
  //   }
  //   else {
  //     Serial.println("Lỗi chuyển chế độ");
  //   }
  //   Serial.println(stMessage);
  // } else if (strcmp(topic, "project/userSetDefaultTemp") == 0) {
  //   Serial.println("Người dùng chỉnh nhiệt độ mặc định");
  //   defaultTemperature = stMessage.toInt();
  //   Serial.println(defaultTemperature);
  // } else if (strcmp(topic, "project/userSetDefaultHumid") == 0) {
  //   Serial.println("Người dùng chỉnh độ ẩm mặc định");
  //   defaultHumidity = stMessage.toInt();
  //   Serial.println(defaultHumidity);
  // } else if (strcmp(topic, "project/userSetDefaultValues") == 0) {
  //   Serial.println("Người dùng chỉnh về thông số mặc định");
  //   defaultTemperature = 35;
  //   defaultHumidity = 80;
  // } else {
  //   Serial.println("error2");
  // }
}

void firebaseConnect() {
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.api_key = API_KEY;
  
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void sendHttpRequest(char* request){
  WiFiClient client;
  while(!client.connect(host,port)){
    Serial.println("Connection fail");
    delay(1000);
  }
  client.print("GET "+String(request)+" HTTP/1.1\r\n"
  +"Host: "+host+"\r\n"+
  "Connection: close\r\n\r\n");
  delay(500);
  while(client.available()){
    String line =client.readStringUntil('\r');
    Serial.print(line);
  }
}

void setup(){
  Serial.begin(115200);

  pinMode(RAIN_PIN, INPUT);

  pinMode(MOTOR_PIN_1, OUTPUT);
  pinMode(MOTOR_PIN_2, OUTPUT);
  digitalWrite(MOTOR_PIN_1,LOW);
  digitalWrite(MOTOR_PIN_2,LOW);

  dht.begin();
  
  // pinMode(GPIO_NUM_2, OUTPUT);
  // digitalWrite(GPIO_NUM_2, HIGH);
  wifiConnect();

  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqttServer, port);
  client.setCallback(callback);

  firebaseConnect();

  // get setting from firebase
  while (!Firebase.ready()) {
    Serial.println("Waiting for firebase...");
  };
  if (Firebase.ready()) {
    Serial.println("Get data from firebase\n");
    Firebase.getInt(fbdo, F("/setting/autoMode"));
    autoMode = fbdo.to<int>();
    Firebase.getBool(fbdo, FPSTR("/setting/isAuto"));
    isAuto = fbdo.to<bool>();
    Firebase.getBool(fbdo, FPSTR("/setting/isOpen"));
    isOpen = fbdo.to<bool>();

    Firebase.getInt(fbdo, F("/setting/defaultTemperature"));
    defaultTemperature = fbdo.to<float>();
    Firebase.getInt(fbdo, F("/setting/defaultHumidity"));
    defaultHumidity = fbdo.to<float>();
  };
}

void closeMaiche(){
  digitalWrite(MOTOR_PIN_1,HIGH);
  delay(2000);
  digitalWrite(MOTOR_PIN_1,LOW);
  sendHttpRequest(requestAwningClose);
}

void openMaiche(){
  digitalWrite(MOTOR_PIN_2,HIGH);
  delay(2000);
  digitalWrite(MOTOR_PIN_2,LOW);
  sendHttpRequest(requestAwningOpen);
}

void loop(){
  // mqtt status & reconnect
  if (!client.connected()) {
    mqttReconnect();
  }
  client.loop();

  while (!Firebase.ready()) {
    Serial.println("Waiting for firebase...");
  };
  if (Firebase.ready()) {
    Firebase.getInt(fbdo, F("/setting/autoMode"));
    autoMode = fbdo.to<int>();
    Firebase.getBool(fbdo, FPSTR("/setting/isAuto"));
    isAuto = fbdo.to<bool>();
    Firebase.getBool(fbdo, FPSTR("/setting/isOpen"));
    isOpen = fbdo.to<bool>();

    Firebase.getInt(fbdo, F("/setting/defaultTemperature"));
    defaultTemperature = fbdo.to<float>();
    Firebase.getInt(fbdo, F("/setting/defaultHumidity"));
    defaultHumidity = fbdo.to<float>();
  }

  // read input
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  rain = digitalRead(23);

  isRaining = (rain == LOW);

  // Message publish to MQTT
  char buffer[200];
  if (isnan(temperature) || isnan(humidity)) {
    isHot = false;
    isSafe = true;
    Serial.println("Failed to read from DHT sensor!");
    sprintf(buffer, "{\"defaultTemperature\":%.0f, \"defaultHumidity\":%.0f, \"temperature\":0, \"humidity\":0, \"isOpen\":%s, \"isRaining\":%s, \"isHot\":%s, \"isSafe\":%s, \"isAuto\":%s, \"autoMode\":%d}", defaultTemperature, defaultHumidity, isOpen ? "true" : "false", isRaining ? "true" : "false", isHot ? "true" : "false", isSafe ? "true" : "false", isAuto ? "true" : "false", autoMode);
  }
  else {
    isHot = (temperature > defaultTemperature);
    isSafe = (humidity < defaultHumidity);
    if(isSafe) warning = false;
    sprintf(buffer, "{\"defaultTemperature\":%.0f, \"defaultHumidity\":%.0f, \"temperature\":%.1f, \"humidity\":%.0f, \"isOpen\":%s, \"isRaining\":%s, \"isHot\":%s, \"isSafe\":%s, \"isAuto\":%s, \"autoMode\":%d}", defaultTemperature, defaultHumidity, temperature, humidity, isOpen ? "true" : "false", isRaining ? "true" : "false", isHot ? "true" : "false", isSafe ? "true" : "false", isAuto ? "true" : "false", autoMode);
  }
  // Serial.println(buffer);
  client.publish("project/data", buffer);

  if (isAuto) {
    if (autoMode == 1) {
      if ((isHot && !isOpen)||(isRaining && !isOpen)){
        openMaiche();
        isOpen=true;
      }
      else if (!isHot && isOpen && !isRaining){
        closeMaiche();
        isOpen=false;
      }
    }
    else if (autoMode == 2) {
      // chế độ "Phơi đồ"
      if(isRaining && !isOpen){
        openMaiche();
        isOpen=true;
      }
      else if (!isRaining && isOpen){
        closeMaiche();
        isOpen=false;
      }
    }
  }

  if(!isSafe && !warning){
    sendHttpRequest(requestHumidityWarning);
    warning = true;
  }

  delay(500);
}