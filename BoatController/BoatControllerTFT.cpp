#include "BoatControllerTFT.h"
#include "SPI.h"
#include <FS.h>
#include <ArduinoJson.h>

#include <TFT_eSPI.h>
#define CALIBRATION_FILE "/calibrationData"

TFT_eSPI tft = TFT_eSPI();

// JPEG decoder library
#include <JPEGDecoder.h>
int LoopCount;
unsigned long LastDraw;
extern DynamicJsonDocument config;
File root;

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
    // Set all chip selects high to avoid bus contention during initialisation of each peripheral
    digitalWrite(22, HIGH); // Touch controller chip select (if used)
    digitalWrite(15, HIGH); // TFT screen chip select
    digitalWrite(5, HIGH); // SD card chips select, must use GPIO 5 (ESP32 SS)

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
    this->SlideshownDelay = config["SlideShowDelay"].as<long>() | 10;
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
    

    if (tft.getTouch(&x, &y)) {

        tft.setCursor(5, 5, 2);
        tft.printf("x: %i     ", x);
        tft.setCursor(5, 20, 2);
        tft.printf("y: %i    ", y);

        tft.drawPixel(x, y, color);
        color += 155;
    }

    if (timenow > LastDraw + this->SlideshownDelay) {
        Serial.println("Doing Slideshow...");

        LoopCount++;
        if (root) {
            DoSlideShow();
        }
       
    }
    
}

