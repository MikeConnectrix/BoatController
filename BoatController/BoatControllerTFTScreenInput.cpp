#include "BoatControllerTFTScreenInput.h"
#include "BoatControllerDefines.h"
#include <TFT_eSPI.h>


extern TFT_eSPI tft;

const int NumberofInputKeys = 16;
TFT_eSPI_Button key[NumberofInputKeys];
extern void WriteDebug(String msg);
#define NUM_LEN 12
char numberBuffer[NUM_LEN + 1] = "";
uint8_t numberIndex = 0;


// Create 15 keys for the keypad
char keyLabel[NumberofInputKeys][5] = { "New", "Del", "Send", "1", "2", "3", "4", "5", "6", "7", "8", "9", ".", "0", "#", "Exit" };
uint16_t keyColor[NumberofInputKeys] = { TFT_RED, TFT_DARKGREY, TFT_DARKGREEN,
	TFT_BLUE, TFT_BLUE, TFT_BLUE,
	TFT_BLUE, TFT_BLUE, TFT_BLUE,
	TFT_BLUE, TFT_BLUE, TFT_BLUE,
	TFT_BLUE, TFT_BLUE, TFT_BLUE, TFT_BLUE };

void drawKeypad() {
	// Draw the keys
	tft.fillScreen(0x0000);
	for (uint8_t row = 0; row < 5; row++) {
		for (uint8_t col = 0; col < 3; col++) {
			uint8_t b = col + row * 3;

			if (b < 3)
				tft.setFreeFont(LABEL1_FONT);
			else
				tft.setFreeFont(LABEL2_FONT);

			key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
				KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
				KEY_W, KEY_H, TFT_WHITE, keyColor[b], TFT_WHITE,
				keyLabel[b], KEY_TEXTSIZE);
			key[b].drawButton();
		}
	}
	key[15].initButton(&tft,420,220,100,40, TFT_WHITE, keyColor[15], TFT_WHITE, keyLabel[15], KEY_TEXTSIZE);
	key[15].drawButton();
}

void BoatControllerTFTScreenInputClass::draw() {
	drawKeypad();
}

void BoatControllerTFTScreenInputClass::doWork() {

	uint16_t x, y;
	boolean pressed = tft.getTouch(&x, &y);

	for (uint8_t b = 0; b < 16; b++) {
		if (pressed && key[b].contains(x, y)) {
			key[b].press(true); // tell the button it is pressed
		}
		else {
			key[b].press(false); // tell the button it is NOT pressed
		}
	}

	// Check if any key has changed state
	for (uint8_t b = 0; b < 16; b++) {

		if (b < 3)
			tft.setFreeFont(LABEL1_FONT);
		else
			tft.setFreeFont(LABEL2_FONT);

		if (key[b].justReleased())
			key[b].drawButton(); // draw normal

		if (key[b].justPressed()) {
			key[b].drawButton(true); // draw invert

			// if a numberpad button, append the relevant # to the numberBuffer
			if (b >= 3) {
				if (numberIndex < NUM_LEN) {
					numberBuffer[numberIndex] = keyLabel[b][0];
					numberIndex++;
					numberBuffer[numberIndex] = 0; // zero terminate
				}
			}

			// Del button, so delete last char
			if (b == 1) {
				numberBuffer[numberIndex] = 0;
				if (numberIndex > 0) {
					numberIndex--;
					numberBuffer[numberIndex] = 0; //' ';
				}
			}

			if (b == 2) {
				WriteDebug("Sent value to serial port\n");
				Serial.println(numberBuffer);
			}
			// we dont really check that the text field makes sense
			// just try to call
			if (b == 0) {
				WriteDebug("Value cleared\n");
				numberIndex = 0;			   // Reset index to 0
				numberBuffer[numberIndex] = 0; // Place null in buffer
			}

			// Update the number display field
			tft.setTextDatum(TL_DATUM);		  // Use top left corner as text coord datum
			tft.setFreeFont(&FreeSans18pt7b); // Choose a nicefont that fits box
			tft.setTextColor(DISP_TCOLOR);	  // Set the font colour

			// Draw the string, the value returned is the width in pixels
			int xwidth = tft.drawString(numberBuffer, DISP_X + 4, DISP_Y + 12);

			// Now cover up the rest of the line up by drawing a black rectangle.  No flicker this way
			// but it will not work with italic or oblique fonts due to character overlap.
			tft.fillRect(DISP_X + 4 + xwidth, DISP_Y + 1, DISP_W - xwidth - 5, DISP_H - 2, TFT_BLACK);

			delay(10); // UI debouncing
		}
	}
}

BoatControllerTFTScreenInputClass BoatControllerTFTScreenInput;
