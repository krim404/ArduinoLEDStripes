/*
  simtronyx_RGB_LED.cpp - Library RGB LEDs & Strips.
  Created by Ronny Simon, 2013.
  (c) Ronny Simon
  http://blog.simtronyx.de
*/

#include "Arduino.h"
#include "simtronyx_RGB_LED.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Init

simtronyx_RGB_LED::simtronyx_RGB_LED(int pinR,int pinG,int pinB){
	
	pinRed=pinR;
    pinGreen=pinG;
    pinBlue=pinB;
	
	pinMode(pinRed,OUTPUT);
	pinMode(pinGreen,OUTPUT);
	pinMode(pinBlue,OUTPUT);
	
	setBrightnessRGB();
	
	color0=color1=color2=0;
	colorAdd0=colorAdd1=colorAdd2=0;
	
	animateColorType=ANIMATETYPE_RGB;
	animateColorsClear();
	
	animateSpeed=10;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Loop (if animate is active)

void simtronyx_RGB_LED::loop(){
	
	if((isAnimateActive)&&(animateColorsCount))animateRun();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Brightness R,G,B Values to make Color Correction for different LED Strips
// normally you do not have a clear white light if you set all colors to 255
// with this function you can adjust this behaviour

void simtronyx_RGB_LED::setBrightnessRGB(int r,int g,int b){

	brightness[0] = r;
	brightness[1] = g;
	brightness[2] = b;
	
	isBrightnessAdjustActive=true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Brightness adjust - on/off

void simtronyx_RGB_LED::useBrightnessAdjust(boolean isAdjust){
		
	isBrightnessAdjustActive=isAdjust;
}

//////////////////////////////////////////////////////////////////////////////////////////
// SET RGB VALUES

void simtronyx_RGB_LED::setRGB(int r,int g,int b){

  setR(r);
  setG(g);
  setB(b);
  //SERIALPORT.print(r);SERIALPORT.print(",");SERIALPORT.print(g);SERIALPORT.print(",");SERIALPORT.print(b);SERIALPORT.println("");
}

void simtronyx_RGB_LED::setR(int r){
 
  if(r<0)r=0;
  if(r>255)r=255;

  analogWrite(pinRed,(isBrightnessAdjustActive?r*brightness[0]/256:r));
  color0=r*100;
}

void simtronyx_RGB_LED::setG(int g){
 
  if(g<0)g=0;
  if(g>255)g=255;

  analogWrite(pinGreen,(isBrightnessAdjustActive?g*brightness[1]/256:g));
  color1=g*100;
}

void simtronyx_RGB_LED::setB(int b){
 
  if(b<0)b=0;
  if(b>255)b=255;

  analogWrite(pinBlue,(isBrightnessAdjustActive?b*brightness[2]/256:b));
  color2=b*100;
}

void simtronyx_RGB_LED::writeRGB(int r,int g,int b){
	
	if(r>25500)r=25500;else if(r<0)r=0;
	if(g>25500)g=25500;else if(g<0)g=0;
	if(b>25500)b=25500;else if(b<0)b=0;

  //SERIALPORT.print(r);SERIALPORT.print(",");SERIALPORT.print(g);SERIALPORT.print(",");SERIALPORT.print(b);SERIALPORT.println("");
	
	analogWrite(pinRed,(isBrightnessAdjustActive?r/100*brightness[0]/256:r/100));
	analogWrite(pinGreen,(isBrightnessAdjustActive?g/100*brightness[1]/256:g/100));
	analogWrite(pinBlue,(isBrightnessAdjustActive?b/100*brightness[2]/256:b/100));
}

//////////////////////////////////////////////////////////////////////////////////////////
// SET HSV VALUES

void simtronyx_RGB_LED::setHSV(float h,float s,float v){
  
  int r,g,b;
  
  HSV2RGB(h,s,v,&r,&g,&b);
    
  writeRGB(r,g,b);
}

//////////////////////////////////////////////////////////////////////////////////////////
// ANIMATE FUNCTIONS

void simtronyx_RGB_LED::animateSpeedSet(int v){
	
	if(v<0)v=0;
	animateSpeed=v;
}
void simtronyx_RGB_LED::animateColorTypeSet(byte type){
	
	animateColorType=type;
}


void simtronyx_RGB_LED::animateStart(){
	
	isAnimateActive=true;
	animateCalculateRGB();
}

void simtronyx_RGB_LED::animateStop(){
	
	isAnimateActive=false;
}

void simtronyx_RGB_LED::animateColorsClear(){
	
	animateColorsCount=0;
	animateColorsActual=0;
	animateTimeCount=0;
	isAnimateActive=false;
}

boolean simtronyx_RGB_LED::animateColorAdd(int col1,int col2,int col3,int t,int num){
	
	if((num>=0)&&(num<animateColorsCount)){
		animateColors[num*4+0]=col1*100;
		animateColors[num*4+1]=col2*100;
		animateColors[num*4+2]=col3*100;
		animateColors[num*4+3]=t;
		return true;
	}
	else if(num!=-1)return false;
	else if(animateColorsCount<(int)(MAXANIMATECOLORSARRAY/4)){
	
		animateColors[animateColorsCount*4+0]=col1*100;
		animateColors[animateColorsCount*4+1]=col2*100;
		animateColors[animateColorsCount*4+2]=col3*100;
		animateColors[animateColorsCount*4+3]=t;
		animateColorsCount++;
		return true;
	}
	return false;
}

void simtronyx_RGB_LED::animateColorSet(){
  
    switch(animateColorType){
	
       case ANIMATETYPE_HSV:
	   
         setHSV(color0/100,color1/100,color2/100);
       break;
	   
       case ANIMATETYPE_RGB:
	   
         writeRGB(color0,color1,color2);
       break;
    }
}

void simtronyx_RGB_LED::animateRun(){
  
  if(animateTimeCount-1>0){
    
     color0+=colorAdd0;
     color1+=colorAdd1;
     color2+=colorAdd2;
     
     animateColorSet();
     
     delay(animateSpeed); //  TODO: no delay() maybe millis() ??? important for more than one strip or other functions on chip
     animateTimeCount--;
  }
  else{
    
     color0=animateColors[animateColorsActual*4+0];
     color1=animateColors[animateColorsActual*4+1];
     color2=animateColors[animateColorsActual*4+2];
     
     animateColorSet();
         
     if(animateColorsActual+1<animateColorsCount)animateColorsActual++;
     else animateColorsActual=0;
     
     animateCalculateRGB();
  }
}

void simtronyx_RGB_LED::animateCalculateRGB(){
 
  animateTimeCount=animateColors[animateColorsActual*4+3];
  colorAdd0=(animateColors[animateColorsActual*4+0]-color0)/animateTimeCount;
  colorAdd1=(animateColors[animateColorsActual*4+1]-color1)/animateTimeCount;
  colorAdd2=(animateColors[animateColorsActual*4+2]-color2)/animateTimeCount;
}

//////////////////////////////////////////////////////////////////////////////////////////
// HSV2RGB  h=0.0..360.0 , s=0.0..100.0 , v=0.0..100.0

void simtronyx_RGB_LED::HSV2RGB(float h,float s,float v,int *r,int *g,int *b) {
  
  int i;
  float m, n, f;
 
  h=max(0.0, min(360.0, h));
  s=max(0.0, min(100.0, s));
  v=max(0.0, min(100.0, v));
  
  s/=100;
  v/=100;
  
  if(s==0){
    *r=*g=*b=round(v*25500);
    return;
  }
  
  h/=60;
  i=floor(h);
  f=h-i;
  
  if(!(i&1)){
    f=1-f;
  }
  
  m=v*(1-s);
  n=v*(1-s*f);
  
  switch (i) {
    
    case 0: case 6:
      *r=round(v*25500);*g=round(n*25500);*b=round(m*25500);
    break;
	  
    case 1: 
      *r=round(n*25500);*g=round(v*25500);*b=round(m*25500);
    break;
	  
    case 2: 
      *r=round(m*25500);*g=round(v*25500);*b=round(n*25500);
    break;
	  
    case 3: 
      *r=round(m*25500);*g=round(n*25500);*b=round(v*25500);
    break;
	  
    case 4: 
      *r=round(n*25500);*g=round(m*25500);*b=round(v*25500);
    break;
	  
    case 5: 
      *r=round(v*25500);*g=round(m*25500);*b=round(n*25500);
    break;
  }
}
