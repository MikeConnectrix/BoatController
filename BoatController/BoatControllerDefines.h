//definitions to make servoData[] easier to work with
#define SERVOTARGET 0
#define SERVOCURRENT 1
#define SERVOSPEED 2

//settings for PCA9685 defaults for servos
#define USMIN 80      // This is the rounded 'minimum' microsecond length based on the minimum pulse of 150
#define USMAX 570     // This is the rounded 'maximum' microsecond length based on the maximum pulse of 600
#define SERVO_FREQ 60  // Analog servos run at ~50 Hz updates

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define STOP 0

#define FORMAT_SPIFFS_IF_FAILED true

// Hardware Serial 2 pins
#define RXD2 16
#define TXD2 17

// SBUS 2 pins
#define SBUSRX 14
#define SBUSTX 34

#define PWMPin1 14
#define PWMPin2 35
#define PWMPin3 32
#define PWMPin4 33


#define LED_1 27
#define LED_2 12

#define GPS_BAUDRATE 9600 // The default baudrate of NEO-6M is 9600
#define IBUS_BAUDRATE 115200 // The default baudrate of NEO-6M is 9600
#define FirmwareVersion "2.8"
#define WebUpdateHost "https://raw.githubusercontent.com/MikeConnectrix/BoatController/refs/heads/master/Updates/"
#define WebUpdatePath "BoatController29.bin"
#define WebFilesUpdateManifest "webpages/manifest27.json"

// Keypad start position, key sizes and spacing
#define KEY_X 40 // Centre of key
#define KEY_Y 96
#define KEY_W 62 // Width and height
#define KEY_H 30
#define KEY_SPACING_X 18 // X and Y gap
#define KEY_SPACING_Y 20
#define KEY_TEXTSIZE 1 // Font size multiplier

// Using two fonts since numbers are nice when bold
#define LABEL1_FONT &FreeSansOblique12pt7b // Key label font 1
#define LABEL2_FONT &FreeSansBold12pt7b	   // Key label font 2

// Numeric display box size and location
#define DISP_X 1
#define DISP_Y 10
#define DISP_W 238
#define DISP_H 50
#define DISP_TSIZE 3
#define DISP_TCOLOR TFT_CYAN










