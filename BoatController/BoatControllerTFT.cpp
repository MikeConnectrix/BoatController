#include "BoatControllerTFT.h"
#include "BoatControllerDefines.h"
#include "BoatControllerWifi.h"
#include "BoatControllerConfig.h"
#include "SPI.h"
#include <FS.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#define CALIBRATION_FILE "/calibrationData"
#include <list>
#include <HTTPClient.h>
#include <FFat.h>
#include "BoatControllerTFTScreen.h"
#include "BoatControllerTFTScreenInput.h"

TFT_eSPI tft = TFT_eSPI();

// JPEG decoder library
#include <JPEGDecoder.h>
int LoopCount;
unsigned long LastDraw;
extern DynamicJsonDocument config;
extern BoatControllerWififClass bContWifi;
extern BoatControllerConfigClass bContConfig;
extern void WriteDebug(String msg);

BoatControllerTFTScreenInputClass inputScreen;

File root;
const int buttons = 5;

// We have a status line for messages
#define STATUS_X 120 // Centred on this
#define STATUS_Y 65


// Forward declares for sanity
struct TFTButton;
void btnPush(uint16_t btnCommand);
void DrawUtilitiesScreen();
void DrawMainScreen();
void DrawDebugScreen();
	String TFTMessageBuffer = "";
int32_t TextLineY = 15;
	// Invoke the TFT_eSPI button class and create all the button objects

enum TFTButtonState {
	Pushed,
	Released,
	Disabled
};

enum TFTScreens {
	Slideshow = 0,
	MainScreen = 1,
    Utilities=2,
    Debug=3
};


void InitButtons(TFTScreens screen);

TFTScreens CurrentScreen = Slideshow;

struct TFTButton {
	uint16_t left;
	uint16_t top;
	uint16_t width;
	uint16_t height;
	TFTButtonState ButtonState;
	byte cornerRadius;
	String text;
	uint16_t foreColor;
	uint16_t BackColor;
	uint16_t PushedBackColor;
	int command;
	TFTScreens Screen;
    void draw() {
		uint16_t drawForeColor;
		uint16_t drawBackColor;

		switch (ButtonState) {
		case Pushed:
			drawForeColor = foreColor;
			drawBackColor = PushedBackColor;
			break;
		default:
			drawForeColor = foreColor;
			drawBackColor = BackColor;
			break;
		}
		tft.fillRoundRect(
			left,
			top,
			width,
			height,
			cornerRadius,
			drawBackColor);
		tft.setTextSize(1);
		tft.setFreeFont();
		tft.setTextColor(drawForeColor);
		tft.drawString(text, left + (width - tft.textWidth(text)) / 2, top + (height - tft.fontHeight()) / 2);
    }
    void pushed(){
		btnPush(command);
    }
};

TFTButton TFTButtons[buttons];


void DrawScreen(TFTScreens screen) {
	CurrentScreen = screen;
	InitButtons(CurrentScreen);
	Serial.println((String) screen);
    switch (screen) {
	case MainScreen:
		DrawMainScreen();
		break;
	case Utilities:
		DrawUtilitiesScreen();
		break;
	case Debug:
		DrawDebugScreen();
		break;
	}
}

void SendWebRequest(String WebAddress) {
	HTTPClient http;
	String UpdateAddress = "http://" + bContWifi.ControllerIPAddress + WebAddress;
	http.begin(UpdateAddress);
	int httpResponseCode = http.GET();
	http.end();
}

void btnPush(uint16_t btnCommand) {
	switch (btnCommand) {
	case 1:
		DrawScreen(Utilities);
		break;
	case 2:
		inputScreen.draw();
		DrawScreen(Debug);
		break;
	case 3:
		SendWebRequest("/format");
		TFTButtons[btnCommand].ButtonState = Released;
		break;
	case 4:
		SendWebRequest("/updateweb");
		TFTButtons[btnCommand].ButtonState = Released;
		break;
	case 5:
		WriteDebug("Saving config file to SD Card...");
		bContConfig.saveConfigToSDFile("/config.bak");
		TFTButtons[btnCommand].ButtonState = Released;
		break;
	case 6:
		bContConfig.restoreConfigFromSDFile("/config.bak");
		TFTButtons[btnCommand].ButtonState = Released;
		break;
	case 7:
		DrawScreen(MainScreen);
		break;
	default:
		Serial.printf("%s Button Pushed...\n", TFTButtons[btnCommand].text);
	}
}

void InitButtons(TFTScreens screen) {
    switch (screen) {
	case MainScreen:
		TFTButtons[0] = TFTButton{ 294, 36, 150, 38, Released, 5, "Setup", 0x0, 0x555, 0x6340, 0, MainScreen };
		TFTButtons[1] = TFTButton{ 294, 86, 150, 38, Released, 5, "Utilities", 0x0, 0x555, 0x6340, 1, MainScreen };
		TFTButtons[2] = TFTButton{ 294, 136, 150, 38, Released, 5, "Debug", 0x0, 0x555, 0x6340, 2, MainScreen };
		break;
	case Utilities:
		TFTButtons[0] = TFTButton{ 294, 36, 150, 38, Released, 5, "Format FFat Drive", 0x0, 0x555, 0x6340, 3, Utilities };
		TFTButtons[1] = TFTButton{ 294, 86, 150, 38, Released, 5, "Update Web Page", 0x0, 0x555, 0x6340, 4, Utilities };
		TFTButtons[2] = TFTButton{ 294, 136, 150, 38, Released, 5, "Backup Config to SD", 0x0, 0x555, 0x6340, 5, Utilities };
		TFTButtons[3] = TFTButton{ 294, 186, 150, 38, Released, 5, "Restore Config from SD", 0x0, 0x555, 0x6340, 6, Utilities };
		TFTButtons[4] = TFTButton{ 294, 236, 150, 38, Released, 5, "Exit", 0x0, 0x555, 0x6340, 7, Utilities };	
		break;
    case Debug:
		TFTButtons[0] = TFTButton{ 330, 270, 120, 38, Released, 5, "Exit", 0x0, 0x555, 0x6340, 7, Debug };
		break;
	
    }
	
}

void DrawButtons() {
	for (int i = 0; i < buttons; i++) {
		if (TFTButtons[i].Screen== CurrentScreen)
			TFTButtons[i].draw();
    }   
}

void DrawMainScreen() {
	tft.fillScreen(0x0000);
	tft.drawRect(22, 22, 161, 73, 0xAD55);
	tft.drawRect(280, 22, 173, 280, 0xFFFF);    
	tft.setTextSize(1);
	tft.setFreeFont();
	tft.setTextColor(0xFFFF);
	tft.drawString("System Information", 29, 26);
	tft.drawString("FW Version:" + String(FirmwareVersion), 31, 63);
	tft.drawString("IP Address:" + bContWifi.ControllerIPAddress, 29, 44);
	DrawButtons();
}

void DrawUtilitiesScreen() {
	Serial.println("Drawing Utilities");
	tft.fillScreen(0x0000);
	tft.fillRect(24, 24, 277, 257, 0x0000);
	tft.drawRect(22, 22, 250, 280, 0xAD55);
	tft.drawRect(280, 22, 173, 280, 0xFFFF);    
	DrawButtons();	
    TextLineY = 15;
}

void DrawDebugScreen() {
	/*WriteDebug("Drawing Debug");
	tft.fillScreen(0x0000);
	tft.fillRect(22, 24, 438, 258, 0x0000);
	tft.drawRect(20,260, 440, 60, 0xAD55);
	inputScreen.draw();
	DrawButtons();
	TextLineY = 15;*/
}


void showTime(uint32_t msTime) {
    Serial.print(F(" JPEG drawn in "));
    Serial.print(msTime);
    Serial.println(F(" ms "));
}

void jpegRender(int xpos, int ypos) {

    uint16_t* pImg;
    uint16_t mcu_w = JpegDec.MCUWidth;
    uint16_t mcu_h = JpegDec.MCUHeight;
    uint32_t max_x = JpegDec.width;
    uint32_t max_y = JpegDec.height;

    bool swapBytes = tft.getSwapBytes();
    tft.setSwapBytes(true);

    // Jpeg images are draw as a set of image block (tiles) called Minimum Coding Units (MCUs)
    // Typically these MCUs are 16x16 pixel blocks
    // Determine the width and height of the right and bottom edge image blocks
    uint32_t min_w = jpg_min(mcu_w, max_x % mcu_w);
    uint32_t min_h = jpg_min(mcu_h, max_y % mcu_h);

    // save the current image block size
    uint32_t win_w = mcu_w;
    uint32_t win_h = mcu_h;

    // record the current time so we can measure how long it takes to draw an image
    uint32_t drawTime = millis();

    // save the coordinate of the right and bottom edges to assist image cropping
    // to the screen size
    max_x += xpos;
    max_y += ypos;

    // Fetch data from the file, decode and display
    while (JpegDec.read()) {    // While there is more data in the file
        pImg = JpegDec.pImage;   // Decode a MCU (Minimum Coding Unit, typically a 8x8 or 16x16 pixel block)

        // Calculate coordinates of top left corner of current MCU
        int mcu_x = JpegDec.MCUx * mcu_w + xpos;
        int mcu_y = JpegDec.MCUy * mcu_h + ypos;

        // check if the image block size needs to be changed for the right edge
        if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
        else win_w = min_w;

        // check if the image block size needs to be changed for the bottom edge
        if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
        else win_h = min_h;

        // copy pixels into a contiguous block
        if (win_w != mcu_w)
        {
            uint16_t* cImg;
            int p = 0;
            cImg = pImg + win_w;
            for (int h = 1; h < win_h; h++)
            {
                p += mcu_w;
                for (int w = 0; w < win_w; w++)
                {
                    *cImg = *(pImg + w + p);
                    cImg++;
                }
            }
        }

        // calculate how many pixels must be drawn
        uint32_t mcu_pixels = win_w * win_h;

        // draw image MCU block only if it will fit on the screen
        if ((mcu_x + win_w) <= tft.width() && (mcu_y + win_h) <= tft.height())
            tft.pushImage(mcu_x, mcu_y, win_w, win_h, pImg);
        else if ((mcu_y + win_h) >= tft.height())
            JpegDec.abort(); // Image has run off bottom of screen so abort decoding
    }

    tft.setSwapBytes(swapBytes);

    showTime(millis() - drawTime); // These lines are for sketch testing only
}

//####################################################################################################
// Print image information to the serial port (optional)
//####################################################################################################
// JpegDec.decodeFile(...) or JpegDec.decodeArray(...) must be called before this info is available!
void jpegInfo() {

    // Print information extracted from the JPEG file
    Serial.println("JPEG image info");
    Serial.println("===============");
    Serial.print("Width      :");
    Serial.println(JpegDec.width);
    Serial.print("Height     :");
    Serial.println(JpegDec.height);
    Serial.print("Components :");
    Serial.println(JpegDec.comps);
    Serial.print("MCU / row  :");
    Serial.println(JpegDec.MCUSPerRow);
    Serial.print("MCU / col  :");
    Serial.println(JpegDec.MCUSPerCol);
    Serial.print("Scan type  :");
    Serial.println(JpegDec.scanType);
    Serial.print("MCU width  :");
    Serial.println(JpegDec.MCUWidth);
    Serial.print("MCU height :");
    Serial.println(JpegDec.MCUHeight);
    Serial.println("===============");
    Serial.println("");
}

//####################################################################################################
// Draw a JPEG on the TFT pulled from SD Card
//####################################################################################################
// xpos, ypos is top left corner of plotted image
void drawSdJpeg(const char* filename, int xpos, int ypos) {

    // Open the named file (the Jpeg decoder library will close it)
    File jpegFile = SD.open(filename, FILE_READ);  // or, file handle reference for SD library

    if (!jpegFile) {
        Serial.print("ERROR: File \""); Serial.print(filename); Serial.println("\" not found!");
        return;
    }

    Serial.println("===========================");
    Serial.print("Drawing file: "); Serial.println(filename);
    Serial.println("===========================");

    // Use one of the following methods to initialise the decoder:
    boolean decoded = JpegDec.decodeSdFile(jpegFile);  // Pass the SD file handle to the decoder,
    //boolean decoded = JpegDec.decodeSdFile(filename);  // or pass the filename (String or character array)

    if (decoded) {
        // print information about the image to the serial port
        jpegInfo();
        // render the image onto the screen at given coordinates
        jpegRender(xpos, ypos);
    }
    else {
        Serial.println("Jpeg file format not supported!");
    }
	CurrentScreen = Slideshow;
}

void DoSlideShow() {
    
    bool Done = false;
    while (!Done) {
        File photo = root.openNextFile();
        if (!photo) {
            Done = true;
            root.rewindDirectory();			
        }
        else {
            String filename = photo.name();
            if (SD.exists(photo.name()) && filename.endsWith(".JPG")) {
                tft.fillScreen(random(0xFFFF));
                Serial.println(filename);
                drawSdJpeg(photo.name(), 0, 0);
                Done = true;
				LastDraw = millis();
            }
        }
        photo.close();
    }

}

void BoatControllerTFTClass::init()
{
	tft.begin();
	uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
    
    uint16_t calibrationData[5];
    uint8_t calDataOK = 0;
    tft.setRotation(3);  // portrait

    // check if calibration file exists
    if (SD.exists(CALIBRATION_FILE)) {
        File f = SD.open(CALIBRATION_FILE, "r");
        if (f) {
            if (f.readBytes((char*)calibrationData, 14) == 14)
                calDataOK = 1;
            f.close();
        }
    }
    
    if (calDataOK) {
        // calibration data valid
        Serial.println("Set Calibration Data OK");
        tft.setTouch(calibrationData);
    }
    else {
        // data not valid. recalibrate
        tft.calibrateTouch(calibrationData, TFT_WHITE, TFT_RED, 15);
        // store data
        File f = SD.open(CALIBRATION_FILE, "w");
        if (f) {
            f.write((const unsigned char*)calibrationData, 14);
            f.close();
        }
    }
    //Set Slideshow delay if SD card detected. Default to 10 seconds if not there.
	this->SlideshownDelay = config["Params"]["SShowDel"].as<long>() | 10;
	this->SlideshownDelay = this->SlideshownDelay * 1000;
	Serial.printf("Slide show Delay set to %d\n", this->SlideshownDelay);
    root = SD.open("/");
	LastDraw = millis() - (this->SlideshownDelay-2000);

}

void BoatControllerTFTClass::doWork()
{
		unsigned long timenow = millis();
		uint16_t x, y;
		static uint16_t color;

		inputScreen.doWork();
		
		boolean pressed = tft.getTouch(&x, &y);

		if (pressed) {
			if (CurrentScreen == Slideshow)
				DrawScreen(MainScreen);
			else {
				for (int i = 0; i < buttons; i++) {
					TFTButton tb = TFTButtons[i];
					if (tb.left < x && tb.left + tb.width > x && tb.top < y && tb.top + tb.height > y && tb.Screen == CurrentScreen)
						if (tb.ButtonState != Pushed) {
							TFTButtons[i].ButtonState = Pushed;
							TFTButtons[i].draw();
						}
				}				
			}
		}
		else {
			if (CurrentScreen != Slideshow) {
				for (int i = 0; i < buttons; i++) {
					if (TFTButtons[i].ButtonState == Pushed) {
						TFTButtons[i].ButtonState = Released;
						TFTButtons[i].draw();
						if (TFTButtons[i].Screen == CurrentScreen)
							TFTButtons[i].pushed();
					}
				}
			}

            if (!TFTMessageBuffer.equals("")) {
				if (CurrentScreen == Utilities || CurrentScreen == Debug) {
					if (TextLineY > 230) {
						if (CurrentScreen == Utilities)
							tft.fillRect(23, 23, 248, 278, 0x0000);
						else
							tft.fillRect(22, 24, 438, 228, 0x0000);
						TextLineY = 15;
					}
					if (TFTMessageBuffer.endsWith("\n"))
						TextLineY += 12;

					tft.setTextSize(1);
					tft.setFreeFont();
					tft.setTextColor(0xFFFF);

					tft.drawString(TFTMessageBuffer, 29, TextLineY);
				}

				TFTMessageBuffer = "";
            }
            
		}
		

		if (timenow > LastDraw + this->SlideshownDelay && (CurrentScreen==MainScreen || CurrentScreen==Slideshow)) {
			WriteDebug("Doing Slideshow...");

			LoopCount++;
			if (root) {
				DoSlideShow();
			}
		}
    
}


void BoatControllerTFTClass::TFTBuffer(String msg) {
	TFTMessageBuffer = msg;	
}
