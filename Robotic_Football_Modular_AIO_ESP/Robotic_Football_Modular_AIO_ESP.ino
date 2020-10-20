#include "src/esp32-ps3-develop/src/Ps3Controller.h"
#include "src/ESP32Servo/src/ESP32Servo.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>

// #include <EEPROM.h>
#include <Preferences.h>

#include "src/async-mqtt-client/src/AsyncMqttClient.h"
#include "src/ArduinoJson/ArduinoJson-v6.16.1.h"


//===========Uncomment a LED===========================
#include "Leds/Leds.cpp"
//#include "Leds/OldLeds.cpp"

//===========Uncomment a drive train===================
#include "DriveTrains/BasicDrive.cpp"
//#include "DriveTrains/CenterDrive.cpp"
//#include "DriveTrains/SquareOmniDrive.cpp"

//===========Uncomment for tackle sensor===============
#define TACKLE

//===========Uncomment to choose a Position============
//#define WR
//#define Center
//#define QB
//#define Kicker

//===========Uncomment if not using bag motors=========
//#define OldMotors

//===========Uncomment for debug modes=================
//#define SHOW_CONTROLLER_INPUT
//#define SHOW_EXECUTION_TIME

//===================================
//Includes the right peripheral file for specified position
#ifdef WR
    #define PERIPHERAL
    #include "Peripherals/WRPeripheral.cpp"
#endif

#ifdef Center
    #define PERIPHERAL
    #include "Peripherals/CenterPeripheral.cpp"
#endif

#ifdef QB
    #define PERIPHERAL
    #include "Peripherals/QBPeripheral.cpp"
#endif

#ifdef Kicker
    #define PERIPHERAL
    #include "Peripherals/KickerPeripheral.cpp"
#endif

//This just enables and disables the old motors
#ifdef OldMotors
  int motorType = -1;
#else
  int motorType = 1;
#endif
//===================================

// Tackle sensor is wired to pin 13
#define TACKLE_INPUT 13           
bool hasIndicated = false;
bool stayTackled = false;

// Default handicap is 3 with no kids mode
int handicap = 3;       
bool kidsMode = false;

// Define joystick variables and other useful variables
int leftX, leftY, rightX, rightY;
int newconnect = 0;
ps3_cmd_t cmd;
float exeTime;

// Change name and contoller address for each robot
char name[4];
char* macaddress;

// Read battery is analog read pin 35

// Name and password of the WiFi network to connect to
//const char* ssid = "RoboticFootballRasPi";
//const char* password = "FootballRobots";
const char* ssid = "PHILIP-DESKTOP";
const char* password = "18o06(W6";
WiFiClient wifiClient;
//WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;

#define MQTT_HOST IPAddress(192, 168, 137, 223)
#define MQTT_PORT 1883

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

/**
 * Connect to WiFi
 */
void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);
}
/**
 * Connect to the MQTT server
 */
void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

/**
 * Callback function for a WiFiEvent
 * @param event The WiFi event
 */
void WiFiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        connectToMqtt();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
		    xTimerStart(wifiReconnectTimer, 0);
        break;
    }
}




// void WiFiEvent(WiFiEvent_t event)
// {
//     Serial.printf("[WiFi-event] event: %d\n", event);
//
//     switch (event) {
//         case SYSTEM_EVENT_WIFI_READY: 
//             Serial.println("WiFi interface ready");
//             break;
//         case SYSTEM_EVENT_SCAN_DONE:
//             Serial.println("Completed scan for access points");
//             break;
//         case SYSTEM_EVENT_STA_START:
//             Serial.println("WiFi client started");
//             break;
//         case SYSTEM_EVENT_STA_STOP:
//             Serial.println("WiFi clients stopped");
//             break;
//         case SYSTEM_EVENT_STA_CONNECTED:
//             Serial.println("Connected to access point");
//             break;
//         case SYSTEM_EVENT_STA_DISCONNECTED:
//             Serial.println("Disconnected from WiFi access point");
//             break;
//         case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
//             Serial.println("Authentication mode of access point has changed");
//             break;
//         case SYSTEM_EVENT_STA_GOT_IP:
//             Serial.print("Obtained IP address: ");
//             Serial.println(WiFi.localIP());
//             break;
//         case SYSTEM_EVENT_STA_LOST_IP:
//             Serial.println("Lost IP address and IP address is reset to 0");
//             break;
//         case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
//             Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
//             break;
//         case SYSTEM_EVENT_STA_WPS_ER_FAILED:
//             Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
//             break;
//         case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
//             Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
//             break;
//         case SYSTEM_EVENT_STA_WPS_ER_PIN:
//             Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
//             break;
//         case SYSTEM_EVENT_AP_START:
//             Serial.println("WiFi access point started");
//             break;
//         case SYSTEM_EVENT_AP_STOP:
//             Serial.println("WiFi access point  stopped");
//             break;
//         case SYSTEM_EVENT_AP_STACONNECTED:
//             Serial.println("Client connected");
//             break;
//         case SYSTEM_EVENT_AP_STADISCONNECTED:
//             Serial.println("Client disconnected");
//             break;
//         case SYSTEM_EVENT_AP_STAIPASSIGNED:
//             Serial.println("Assigned IP address to client");
//             break;
//         case SYSTEM_EVENT_AP_PROBEREQRECVED:
//             Serial.println("Received probe request");
//             break;
//         case SYSTEM_EVENT_GOT_IP6:
//             Serial.println("IPv6 is preferred");
//             break;
//         case SYSTEM_EVENT_ETH_START:
//             Serial.println("Ethernet started");
//             break;
//         case SYSTEM_EVENT_ETH_STOP:
//             Serial.println("Ethernet stopped");
//             break;
//         case SYSTEM_EVENT_ETH_CONNECTED:
//             Serial.println("Ethernet connected");
//             break;
//         case SYSTEM_EVENT_ETH_DISCONNECTED:
//             Serial.println("Ethernet disconnected");
//             break;
//         case SYSTEM_EVENT_ETH_GOT_IP:
//             Serial.println("Obtained IP address");
//             break;
//         default: break;
//     }}






/**
 * Callback function for when connected to the MQTT server
 * @param sessionPresent Wheather the session is present or not
 */
void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  uint16_t packetIdSub = mqttClient.subscribe("esp32/input", 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);
  mqttClient.publish("esp32/output", 0, true, "test 1");
  Serial.println("Publishing at QoS 0");
  uint16_t packetIdPub1 = mqttClient.publish("esp32/output", 1, true, "test 2");
  Serial.print("Publishing at QoS 1, packetId: ");
  Serial.println(packetIdPub1);
  uint16_t packetIdPub2 = mqttClient.publish("esp32/output", 2, true, "test 3");
  Serial.print("Publishing at QoS 2, packetId: ");
  Serial.println(packetIdPub2);
}

/**
 * Callback function for when the MQTT client is disconnected from the server
 * @param reason The reason why the mqtt client was disconnected
 */
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

/**
 * Callback function for acknowledging a subscription to a topic
 */
void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

/**
 * Callback function for acknowledging an unsubscription from a topic
 */
void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

/**
 * Callback function for when a message is recieved from the MQTT server
 * @param topic The topic that the message was recieved on
 * @param payload The actual message itself
 * @param properties If the message has any properties
 * @param len The length of the message
 * @param index Index
 * @param total Total
 */
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print(" payload: ");
  Serial.println(payload);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);

  char messageCommand;
  String messageTemp;
  String messageInput;

  for (int i = 0; i < len; i++) {
    Serial.print((char)payload[i]);
    messageTemp += (char)payload[i];
  }

  if (String(topic) == "esp32/input") {
    //Serial.print("Changing output to ");

    //payload = messageTemp[0]; // Maybe change up to pointers to make more efficient?
    int index = 0;
    for (int i = 1; i < len; i++) if (messageTemp[i] == '-') index = i + 1;
    for (int i = index; i < len; i++) messageInput += messageTemp[i];
    
    if(messageTemp == "p"){
      Serial.println("Reprogramming");
      update();
    }
    else if(messageCommand == 'r'){
      readStoredName();
      Serial.print("Name read from EEPROM:");
      Serial.print(name);
      Serial.print("\n");
    }
    else if(messageCommand == 'w'){
      writeStoredName(messageInput);
      Serial.print("Name written to EEPROM: ");
      Serial.print(messageInput);
      Serial.print("\n");
    }
    else {
      Serial.println("Invalid message");
    }
  }
}

/**
 * Callback function to ackknowledge the client sending a message
 * @param packetID The ID of the message that was sent
 */
void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}


const char* robotNames[18] = {"r3",  "r7",  "r9", 
                          "r12", "r16", "r35", 
                          "r40", "r42", "r44", 
                          "r53", "r68", "r74", 
                          "r75", "r81", "r82", 
                          "r85", "r88", "rK9"};

char* macAddresses[18] = {"00:15:83:f3:e8:d8", "00:15:83:3d:0a:57", "00:1b:dc:0f:aa:58", 
                             "00:1b:dc:0f:f3:59", "5c:f3:70:78:51:d6", "00:1b:dc:0f:d3:f1", 
                             "00:1b:dc:0f:f3:59", "00:15:83:f3:e0:09", "00:1b:dc:0f:dc:32", 
                             "00:1b:dc:0f:dc:3e", "00:1b:dc:0f:d3:e8", "5c:f3:70:78:51:d0", 
                             "01:02:03:04:05:06", "00:15:83:f3:e8:cd", "5c:f3:70:6e:5f:df", 
                             "00:1b:dc:0f:dc:2d", "00:1b:dc:0f:e8:af", "00:15:83:f3:e8:e8"};


// JSON variables for sending data to webserver
const int capacity = JSON_OBJECT_SIZE(6);
StaticJsonDocument<capacity> data;
char buffer[256];

// Contoller battery level
int battery = 0;

#define EEPROM_SIZE 16

Preferences preferences;

void writeStoredName(String data) {
  preferences.begin("RobotName");
  preferences.putString("name", data);
  preferences.end();
}

void readStoredName() {
  preferences.begin("RobotName", false);
  String data = preferences.getString("name");
  preferences.end();

  Serial.print("Read name: ");
  Serial.print(data);
  Serial.print("\n");

  if (data.length() == 3) {
    strcpy(name, data.c_str());
  }

  Serial.print("Read name: ");
  Serial.print(name);
  Serial.print("\n");

  for (int i = 0; i < 18; i++) { // Change 18 to make work with any length mac address array
    if (strcmp(name, robotNames[i]) == 0) {
      macaddress = macAddresses[i];
      Serial.println(robotNames[i]);
      Serial.println(macaddress);
      break;
    }
  }
}


/**
 * Updates the esp32 code over http by connecting to a remote webserver
 */
void update() {
  mqttClient.disconnect();
  Serial.println("MQTT disconnected");

  t_httpUpdate_return ret = httpUpdate.update(wifiClient, "http://192.168.137.211:8080/public/esp32.bin");
  
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
  }
}

/**
 * Rumbles the right and left of the contoller based on the given intensity and length values. 
 * 
 * @param rightDuration The duration of the right side rumble in ms
 * @param rightPower The strength of the right side rumble 
 * @param leftDuration The duration of the left side rumble in ms
 * @param leftPower The strength of the left side rumble
 */
void rumbleContoller(uint8_t rightDuration, uint8_t rightPower, uint8_t leftDuration, uint8_t leftPower) {
    cmd.rumble_right_duration = rightDuration;
    cmd.rumble_right_intensity = rightPower;
    cmd.rumble_left_intensity = leftPower;
    cmd.rumble_left_duration = leftDuration;

    ps3Cmd(cmd);
}

/**
 * Callback function for when the contoller connects. This will vibrate the contoller only on the first connection after the robot is turned on.
 */
void onControllerConnect(){
    // Vibrates controller when you connect
    if (newconnect == 0) {
      rumbleContoller(50, 255, 50, 255);
      newconnect++;
      ps3SetLed(1);
    }

    Serial.println("Controller is connected!");
}




void setup() {// This is stuff for connecting the PS3 controller.
  Serial.begin(115200);       //Begin Serial Communications
  ledsSetup();          //Setup the leds
  flashLeds();
  Serial.println("ESP32 starting up");
  String dadfa = "rK9";
  writeStoredName(dadfa);
  readStoredName();

  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToWifi();


  // Attached the contoller connect function to connection callback and start contoller connection
  Ps3.attachOnConnect(onControllerConnect);
  Ps3.begin(macaddress);
  // while(!Ps3.isConnected()){
  //   Serial.println("Controller not connected");
  //   blue();
  // }
  
  //Setup the drive train, peripherals, tackle sensor, and changes leds to green once complete
  driveSetup(motorType);

  #ifdef PERIPHERAL
    peripheralSetup();
  #endif

  #ifdef TACKLE
    pinMode(TACKLE_INPUT, INPUT);
  #endif

  #ifdef TACKLE
      green();
  #else
      blue();
  #endif   
}

void loop() {
  #ifdef SHOW_EXECUTION_TIME
    exeTime = micros();
  #endif

  data["tackleStatus"] = " ";

  if (WiFi.status() == WL_CONNECTED){
    data["espMacAddress"] = WiFi.macAddress();
  }

  // Run if the controller is connected
  if (Ps3.isConnected()) {
    data["contollerStatus"] = "Connected";
    
    // if( battery != Ps3.data.status.battery ){
    //     battery = Ps3.data.status.battery;
    //     Serial.print("The controller battery is ");
    //     if( battery == ps3_status_battery_charging )      Serial.println("CHARGING");
    //     else if( battery == ps3_status_battery_full )     Serial.println("FULL");
    //     else if( battery == ps3_status_battery_high )     Serial.println("HIGH");
    //     else if( battery == ps3_status_battery_low)       Serial.println("LOW");
    //     else if( battery == ps3_status_battery_dying )    Serial.println("DYING");
    //     else if( battery == ps3_status_battery_shutdown ) Serial.println("SHUTDOWN");
    //     else Serial.println("UNDEFINED");
    // }

    // // Press the PS button to disconnect the controller
    // if (Ps3.data.button.ps && newconnect == 1) {
    // PS3.disconnect();
    //   newconnect = 0;
    // }

    //====================Get Controller Input=================================
    // Reads and maps joystick values from -90 to 90
    leftX = map(Ps3.data.analog.stick.lx, -128, 127, -90, 90);
    leftY = map(Ps3.data.analog.stick.ly, -128, 127, -90, 90);
    rightX = map(Ps3.data.analog.stick.rx, -128, 127, -90, 90);
    rightY = map(Ps3.data.analog.stick.ry, -128, 127, -90, 90);
    
    // Deals with stickness from joysticks
    if (abs(leftX) < 8) leftX = 0;
    if (abs(leftY) < 8) leftY = 0;
    if (abs(rightX) < 8) rightX = 0;
    if (abs(rightY) < 8) rightY = 0;
    
    #ifdef SHOW_CONTROLLER_INPUT
      Serial.print(leftX);    
      Serial.print("\t");
      Serial.print(leftY);    
      Serial.print("\t");
      Serial.print(rightX);    
      Serial.print("\t");
      Serial.print(rightY);    
      Serial.print("\n");
    #endif

    //====================Specify the handicap=================================
    //Toggle in and out of kidsmode
    if (Ps3.data.button.start) {
      if (kidsMode == true) {
        kidsMode = false;
        handicap = 3;
        ps3SetLed(5);     // ON OFF OFF ON          
        rumbleContoller(5, 255, 5, 255);      // vibrate both, then left, then right
      } else if (kidsMode == false) {
        kidsMode = true;
        handicap = 7;
        ps3SetLed(1);     // OFF OFF OFF ON
        rumbleContoller(5, 255, 5, 255);      // vibrate both, then left, then right
      }
    }

    if (kidsMode == false) {
      // Press R2 to boost
      if (Ps3.data.button.r2) {
        handicap = 1;
      }
      // Press L2 to slow down
      else if (Ps3.data.button.l2) {
        handicap = 6;
      } 
      // Sets default handicap
      else {
        handicap = 3;
      }
    }

    //===============================Adjust motors=============================
    if (Ps3.data.button.left){
      correctMotor(1);
      Serial.println("Left button clicked");
      rumbleContoller(0, 0, 5, 255);
    }
    if (Ps3.data.button.right){
      correctMotor(-1);
      Serial.println("Right button clicked");
      rumbleContoller(5, 255, 0, 0);
    }

    //=================================Tackle Sensor===========================

    #ifdef TACKLE
      // NORMAL OPERATION MODE
      // for the if statement for whether or not
      // tackle is enabled. cool stuff
      if (Ps3.data.button.left) {
        if (stayTackled == true) {
          stayTackled = false;
        } 
        else {
          stayTackled = true;
        }
        rumbleContoller(30, 255, 30, 255);
      }
      if (!digitalRead(TACKLE_INPUT))
      {
        red();
        data["tackleStatus"] = "Tackled";
        if (!hasIndicated) {
          rumbleContoller(10, 255, 10, 255);
          hasIndicated = true;
        }
      }
      else
      {
        if (stayTackled == false) {
          hasIndicated = false;
          green();
          data["tackleStatus"] = "Not tackled";
        }
      }

    #endif
    //===============================================================================================

    // Drives the robot according to joystick input
    driveCtrl(handicap, leftX, leftY, rightX, rightY);

    #ifdef PERIPHERAL
        peripheral(Ps3);//Call the peripheral
    #endif
  }
  else { // If the controller is not connected, LEDs blue and stop robot
    blue();
    driveStop();
    data["contollerStatus"] = "Disconnected";
  } 

  #ifdef SHOW_EXECUTION_TIME
    Serial.print("Exe exeTime: ");
    Serial.println(micros() - exeTime);
  #endif

  // // Stores the current time
  // now = millis();


  // // if (WiFi.status() != WL_CONNECTED) {
  //   lastWiFiAttempt = now;
  //   //Serial.println("WiFi Disconnected");
  // } 
  // else if (!mqttClient.connected()) {
  //   //Serial.println("Trying to connect to MQTT");
  //   lastMQTTAttempt = now;
  //   //reconnect();
  // } 
  // else if (now  - lastMsg > 200) {
  //   lastMsg = now;

  //   data["robotNumber"] = name;
  //   data["batteryLevel"] = "22";

  //   size_t n = serializeJson(data, buffer);

  //   // Print json data to serial port
  //   serializeJson(data, Serial);

  //   if (mqttClient.publish("esp32/output", 2, false, buffer)) {
  //     Serial.print(" Success sending message\n");
  //   }
  //   else {
  //     Serial.print(" Error sending message\n");
  //   }
  // }

}
