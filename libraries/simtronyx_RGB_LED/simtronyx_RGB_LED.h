/*
  simtronyx_RGB_LED.h - Library for RGB LEDs & Strips.
  Created by Ronny Simon, 2013.
  (c) Ronny Simon
  http://blog.simtronyx.de
*/

#ifndef SIMTRONYX_RGB_LED_h
#define SIMTRONYX_RGB_LED_h

// Add the possibility to use other Serial-Ports (for instance MEGA or Leonardo)
#ifndef SERIALPORT
#define SERIALPORT Serial
#endif

#include "Arduino.h"

#define ANIMATETYPE_RGB 1
#define ANIMATETYPE_HSV 2

#define MAXANIMATECOLORSARRAY 100  // 25 (MaxColors) * 4 (r,g,b,time OR h,s,v,time)

class simtronyx_RGB_LED{

  public:
    simtronyx_RGB_LED(int pinR,int pinG,int pinB);
	
	void loop();

	void setBrightnessRGB(int r=256,int g=256,int b=256);
	void useBrightnessAdjust(boolean isAdjust);
	
	void setRGB(int r,int g,int b);
	void setR(int r);
	void setG(int g);
	void setB(int b);
	
	void setHSV(float h,float s,float v);
	
	void animateStart();
	void animateStop();
	
	void animateColorsClear();
	boolean animateColorAdd(int col1,int col2,int col3,int t,int num=-1);
	
	void animateSpeedSet(int speed);
	void animateColorTypeSet(byte type);
	
	boolean isBrightnessAdjustActive;
	
  private:
    int pinRed;
    int pinGreen;
    int pinBlue;
	/*
	void writeR(int v);
	void writeG(int v);
	void writeB(int v);
	*/
	void writeRGB(int r,int g,int b);
	
	void animateColorSet();
	void animateCalculateRGB();
	void animateRun();

	void HSV2RGB(float h,float s,float v,int *r,int *g,int *b);	
	
	int color0;
	int color1;
	int color2;
	int colorAdd0;
	int colorAdd1;
	int colorAdd2;
	
	boolean isAnimateActive;
	
	int animateColors[MAXANIMATECOLORSARRAY];
	int animateColorsActual;
	int animateColorsCount;
	
	byte animateColorType;
	int animateSpeed;
	
	long animateTimeCount;
	
	int brightness[3];
};

#endif // SIMTRONYX_RGB_LED_h