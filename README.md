I haven't done a public GitHub share before so please bear with me.

This project is an ESP32 based model RC boat controller.

The hardware is complete, this is the firmware side of things:

![image](https://github.com/user-attachments/assets/f4841db6-e09f-4363-8c37-c67e5d611595)

This unit is designed to be mounted inside an RC model boat. There are inputs for a remote control unit either with direct digital channel inputs or a single 16 channel S-Bus input.

The firmware allows the channels to be mapped to the servo outputs on the board allowing direct control of the assigned channel by the RC transmitter although the individual channels are controlled by the ESP32 MCU.

There is also a Web Page interface to access the controls via the ESP32's WIFI and web server.

The Servo controller is compatible with the Adafruit PWM servo board so can be daisy chained via the I2C header exposed on the control board (Single row yellow 5 pins). Technically up to 32 boards can be connected allowing for access to 992 servos, but the power requirements would limit this somewhat.

The blue 11 row header allows access to a TFT with touchscreen and SD-Card chip select lines. I have used this successfully with the standard IL9144 3.5 inch TFT commonly available.

The unit allows manipulation of the rudder and throttle settings for an RC ship, but also allows rangefinders to be set and moved by the onboard compass. Individual turrets can be assigned to rangefinders and their movement controlled by the range finder they are assigned to.

Practically this means that 2 channels of the RC transmitter can be assigned to the throttle and rudder. Slider controls can be assigned to typically 2 range finder servos (fore and aft). Once set, as the boat turns the rangefinders will track the direction they have been set to. Any turrets assigned to the individual rangefinders, will track the rangefinder.

So as the boat moves through the water, the turrets will remain trained on whatever they have been targeted to via the rangefinders. If the target moves out of scope for the turret (ie superstructure in the way/servo limits reached) the turrets will return to battery position until the target returns to scope.

The configuration file allows for individual servo movement speeds so rate of turn for indiividual turrets can be set to something approaching realistic levels.

The configuration file is a relatively simple json document that can be downloaded, altered and uploaded back through the ESP32's web interface.

Changes Log:

* 7/Nov/2024
> * Updated servo Types input to allow greater number of defined servos
> * Add IBus integration      
