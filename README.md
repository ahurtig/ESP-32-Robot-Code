Robotic-Football-All-In-One
=====

This is the single-file code for the robotic football team. The design philosophy behind this project is to include all of the code for all of the robots inside a single file to make it easy to keep track of the different robots and their shared abilities. The abilities of the robot to be programmed are then chosen from a list of define statements at the top of the file, like so:

````c++
//#define BASIC_DRIVETRAIN
#define OMNIWHEEL_DRIVETRAIN
//#define CENTER_PERIPHERALS
#define QB_PERIPHERALS
//#define KICKER_PERIPHERALS
//#define RECEIVER_PERIPHERALS
#define LED_STRIP
#define TACKLE
#define ROTATION_LOCK
````
This example would create a robot that uses an omniwheel drivetrain (specifically our quarterback), with the led strip and tackle sensor enabled. This robot is also using the new rotation locking compass. At the end of the file are a list of error cases that the compile goes through to make sure no incompatable options are shown.

#### Useful Libraries
---
1. (PS3 Integration) [USB HostShield 2.0](https://github.com/felis/USB_Host_Shield_2.0)
2. (Omniwheel Rotation Locking) [Adafruit BNO055](https://github.com/adafruit/Adafruit_BNO055)

#### Controls
---
  - **Basic Drivetrain**
    - _Up/Down Left Joystick_ - Forward and Backward movement
    - _Left/Right Right Joystick_ - Turning
    - _R2_ - activates "boost"
    - _Start_ - Puts robot in "kids mode". The speed is reduced, boost is disabled, and the leds will change
    - _Select _- Calibration mode - disables drivetrain while changes are made
      - _Up/Down D-Pad_ - compensates for drag left or right
      - _Select_ - exit Calibration Mode to regular drive mode
  - **Center**
    - Center currently uses basic drivetrain
    - _TRIANGLE_ - raise the center release servo
    - _CROSS_ - lower the center release servo
  - **Omniwheel Drivetrain**
    - _Up/Down/Left/Right Left Joystick_ - Lateral movement in any direction
    - _Up/Down/Left/Right D-Pad_- Lateral Movement along compass directions at full power
    - _Left/Right Right Joystick_ - Turning - as of version 1.0.3 this will disable rotation correction
    - _R3_ (Right Joystick Press) - Re-engage rotation correction
    - _R2_ - slow down speed
    - _L1_ - reverse directions (make back of robot front and vise versa)
    - Throwing
      - _SQUARE_ - Handoff throw
      - _CROSS_ - Reciever handoff throw/weak toss
      - _CIRCLE_ - mid range throw
      - _TRIANGLE_ - max power throw
      - _R1_ - return thrower to down position
      - _L2_ - hold to enable throw offset 
        - _Up/Down D-Pad_ - adjust power of all throws but triangle
  - **Kicker**
    - Kicker - currently uses basic drivetrain
    - _CROSS_ - kick
    - _TRIANGLE_ - reload

<!-- USEFUL LINKS -->
## Useful Links

This is a list of useful links and tutorials that were used.
* [Setting up a Raspberry Pi headless](https://www.raspberrypi.org/documentation/configuration/wireless/headless.md)
* [Setting up a Raspberry Pi as a routed wireless access point](https://www.raspberrypi.org/documentation/configuration/wireless/access-point-routed.md)
* [Introduction to IoT: Build an MQTT Server Using Raspberry Pi](https://appcodelabs.com/introduction-to-iot-build-an-mqtt-server-using-raspberry-pi)
* [Friendly toggle to enable/disable Raspberry Pi's as an access point](https://www.raspberrypi.org/forums/viewtopic.php?t=266214)
* [Node.js and Raspberry Pi - Webserver with WebSocket](https://www.w3schools.com/nodejs/nodejs_raspberrypi_webserver_websocket.asp)
* [mqtt-panel](https://github.com/fabaff/mqtt-panel)
* [Node.js + Nginx](https://stackoverflow.com/questions/5009324/node-js-nginx-what-now)
* [How to use ArduinoJson with PubSubClient?](https://arduinojson.org/v6/how-to/use-arduinojson-with-pubsubclient/)
* [WiFiClientEvents.ino](https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiClientEvents/WiFiClientEvents.ino)
* [async-mqtt-client](https://github.com/marvinroger/async-mqtt-client)


## TODO
* Replace EEPROM with esp32-Preferences
* Replace pubsubclient with async-mqtt-client