//definitions to make servoData[] easier to work with
#define SERVOTARGET 0
#define SERVOCURRENT 1
#define SERVOSPEED 2

//settings for PCA9685 defaults for servos
#define USMIN 600      // This is the rounded 'minimum' microsecond length based on the minimum pulse of 150
#define USMAX 2400     // This is the rounded 'maximum' microsecond length based on the maximum pulse of 600
#define SERVO_FREQ 50  // Analog servos run at ~50 Hz updates

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

#define LED_1 27
#define LED_2 12

#define GPS_BAUDRATE 9600  // The default baudrate of NEO-6M is 9600






