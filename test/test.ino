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
bool isOpen;                // Mái che đang đóng
bool isRaining;             // Trời không mưa     (rain = HIGH)
bool isHot ;                // Trời không nóng    (temperature > 35)
bool isSafe ;               // Mạch điện an toàn (humidity < 80)
                   
bool warning;
int autoMode; // 1: Chế độ tự động "Bình thường", 2: Chế độ tự động "Phơi đồ", 3: Người dùng tự điều khiển
int isAuto;

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
const char* ssid = "The Simple Cafe FL1";
const char* password = "simpleisthebest";

// Wifi Lien
// const char* ssid = "YenLien";
// const char* password = "ngoquocanh";

// Wifi 3G
// const char* ssid = "esp32";
// const char* password = "mylinh123";

// Wifi Thư viện
// const char* ssid = "HCMUS Thu Vien";
// const char* password = "12345678";

// const char* ssid = "Thuy Phan";
// const char* password = "linhphan";

const char*  mqttServer = "broker.hivemq.com";
int port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

void wifiConnect() {
  Serial.println("Connecting to WiFi");
  WiFi.mode (WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Esp_wifi_set_ps(WIFI_PS_NONE);
  // bool connect = false;
  // int count = 0;
  // while (!connect) {
  //   if (count % 3 == 0) {
  //     ssid = "The Simple Cafe";
  //     password = "simpleisthebest"; 
  //     Serial.println(ssid);
  //   }
  //   else if (count % 3 == 1) {
  //     ssid = "esp32";
  //     password = "mylinh123"; 
  //     Serial.println(ssid);
  //   }
  //   else {
  //     ssid = "Lien";
  //     password = "iotnhom5";
  //     Serial.println(ssid);
  //   }
  //   WiFi.begin(ssid, password);
  //   int wait = 0;
  //   while (WiFi.status() != WL_CONNECTED && wait < 30) {
  //     delay(500);
  //     Serial.print(".");
  //     wait++;
  //   }
  //   if (WiFi.status() == WL_CONNECTED) {
  //     connect = true;
  //     Serial.println(" Connected!");
  //   } 
  //   else {
  //     count = (count + 1) % 3;
  //   }
  // }
}

void mqttReconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("21127341")) {
      Serial.println(" connected");
      client.subscribe("project/userChangeAutoMode");
      client.subscribe("project/userOpenAwning");
      client.subscribe("project/userChangeMode");
      client.subscribe("project/userSetDefaultTemperature");
      client.subscribe("project/userSetDefaultHumidity");
      client.subscribe("project/userSetDefaultSetting");
      client.subscribe("project/initializeValues");
      // subcribe other topics here

    } else {
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  String stMessage;
  for (int i = 0; i < length; i ++) {
    stMessage += (char)message[i];
  }
  Serial.println(topic);
  
  if (strcmp(topic, "project/initializeValues") == 0) {
    int pos = stMessage.indexOf(";");
    defaultTemperature = stMessage.substring(0, pos).toInt();
    stMessage = stMessage.substring(pos + 1);
    Serial.println(defaultTemperature);

    pos = stMessage.indexOf(";");
    defaultHumidity = stMessage.substring(0, pos).toInt();
    stMessage = stMessage.substring(pos + 1);
    Serial.println(defaultHumidity);

    pos = stMessage.indexOf(";");
    autoMode = stMessage.substring(0, pos).toInt();
    stMessage = stMessage.substring(pos + 1);
    Serial.println(autoMode);

    pos = stMessage.indexOf(";");
    isOpen = stMessage.substring(0, pos) == "true";
    stMessage = stMessage.substring(pos + 1);
    Serial.println(isOpen);

    pos = stMessage.indexOf(";");
    isAuto = stMessage.substring(0, pos) == "true";
    Serial.println(isAuto);
  }
  else if (strcmp(topic, "project/userChangeAutoMode") == 0) {
    if (stMessage == "1") {
      Serial.println("Người dùng chuyển chế độ Bình thường");
      autoMode = 1;
    }
    else if (stMessage == "2") {
      Serial.println("Người dùng chuyển chế độ Phơi đồ");
      autoMode = 2;
    }
    else {
      Serial.println("Lỗi chuyển chế độ tự động");
    }
    Serial.println(stMessage);
  } else if (strcmp(topic, "project/userOpenAwning") == 0) {
    if (stMessage == "true") {
      Serial.println("Người dùng mở mái che");
      openMaiche();
      isOpen = true;
    }
    else if (stMessage == "false") {
      Serial.println("Người dùng đóng mái che");
      closeMaiche();
      isOpen = false;
    }
    else if (stMessage == "inittrue") {
      isOpen = true;
    }
    else if (stMessage == "initfalse") {
      isOpen = false;
    }
    else {
      Serial.println("Lỗi đóng mở mái che");
    }
    Serial.println(stMessage);
  } else if (strcmp(topic, "project/userChangeMode") == 0) {
    if (stMessage == "true") {
      Serial.println("Người dùng chuyển chế độ Tự động");
      isAuto = true;
    }
    else if (stMessage == "false") {
      Serial.println("Người dùng chuyển chế độ Thủ công");
      isAuto = false;
    }
    else {
      Serial.println("Lỗi chuyển chế độ");
    }
    Serial.println(stMessage);
  } else if (strcmp(topic, "project/userSetDefaultTemperature") == 0) {
    Serial.println("Người dùng chỉnh nhiệt độ mặc định");
    defaultTemperature = stMessage.toInt();
    Serial.println(stMessage);
  } else if (strcmp(topic, "project/userSetDefaultHumidity") == 0) {
    Serial.println("Người dùng chỉnh độ ẩm mặc định");
    defaultHumidity = stMessage.toInt();
    Serial.println(stMessage);
  } else if (strcmp(topic, "project/userSetDefaultSetting") == 0) {
    Serial.println("Người dùng chỉnh về thông số mặc định");
    defaultTemperature = 35;
    defaultHumidity = 80;
    Serial.println(stMessage);
  } else {
    Serial.println("Lỗi");
  }
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

void sendHttpRequest(const char* request){
  WiFiClient client;
  while(!client.connect(host,iftttPort)){
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
  Serial.println("ok");
  // pinMode(GPIO_NUM_2, OUTPUT);
  // digitalWrite(GPIO_NUM_2, HIGH);
  wifiConnect();

  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqttServer, port);
  client.setCallback(callback);

  firebaseConnect();
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

  // while (!Firebase.ready()) {
  //   Serial.println("Waiting for firebase...");
  // };

  // if (Firebase.ready()){
  //   Firebase.getInt(fbdo, F("/setting/autoMode"));
  //   autoMode = fbdo.to<int>();

  //   Firebase.getBool(fbdo, FPSTR("/setting/isAuto"));
  //   isAuto = fbdo.to<bool>();

  //   Firebase.getBool(fbdo, FPSTR("/setting/isOpen"));
  //   isOpen = fbdo.to<bool>();

  //   Firebase.getInt(fbdo, F("/setting/defaultTemperature"));
  //   defaultTemperature = fbdo.to<int>();

  //   Firebase.getInt(fbdo, F("/setting/humidityDefault"));
  //   defaultHumidity = fbdo.to<int>();
    
  //   Serial.println("Get from firebase: " + String(defaultTemperature) + " " + String(defaultHumidity) + " " +  String(isAuto) + " " + String(autoMode) + " " + String(isOpen));
  //   // delay(100);
  // }

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
    sprintf(buffer, "{\"temperature\":0, \"humidity\":0, \"isRaining\":%s, \"isAuto\":%s, \"autoMode\":%d}", isRaining ? "true" : "false", isAuto ? "true" : "false", autoMode);
  }
  else {
    isHot = (temperature > defaultTemperature);
    isSafe = (humidity < defaultHumidity);
    if(isSafe) warning = false;
    sprintf(buffer, "{\"temperature\":%.1f, \"humidity\":%.0f, \"isRaining\":%s, \"isAuto\":%s, \"autoMode\":%d}", temperature, humidity, isRaining ? "true" : "false", isAuto ? "true" : "false", autoMode);
  }
  // Serial.println(buffer);
  client.publish("project/data", buffer);

  Serial.println(String(isAuto) + " " + String(autoMode) + " " + String(isHot) + " " + String(isRaining) + " " + String(isOpen) 
  + " " + String(defaultHumidity) + " " + String(defaultTemperature));

  if (isAuto) {
    if (autoMode == 1) {
      if ((isHot && !isOpen)||(isRaining && !isOpen)){
        openMaiche();
        Firebase.setBool(fbdo, F("/setting/isOpen"), true);
        isOpen=true;
      }
      else if (!isHot && isOpen && !isRaining){
        closeMaiche();
        Firebase.setBool(fbdo, F("/setting/isOpen"), false);
        isOpen=false;
      }
    }
    else if (autoMode == 2) {
      // chế độ "Phơi đồ"
      if(isRaining && !isOpen){
        openMaiche();
        Firebase.setBool(fbdo, F("/setting/isOpen"), true);
        isOpen=true;
      }
      else if (!isRaining && isOpen){
        closeMaiche();
        Firebase.setBool(fbdo, F("/setting/isOpen"), false);
        isOpen=false;
      }
    }
  }

  if(!isSafe && !warning){
     Serial.println(String(isAuto) + " " + String(autoMode) + " " + String(isHot) + " " + String(isRaining) + " " + String(isOpen) 
  + " " + String(isSafe) + " " + String(warning) + " " + String(defaultTemperature));
    Serial.println(String(humidity) + " " + String(defaultHumidity));
    sendHttpRequest(requestHumidityWarning);
    warning = true;
  }

  delay(200);
}
