#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "simtronyx_RGB_LED.h"


#define LIGHTID '2'
#define LEDred 5
#define LEDgreen 3
#define LEDblue 6
simtronyx_RGB_LED strip(LEDred,LEDgreen,LEDblue);

int rot,gruen,blau,currot,curgruen,curblau;
int timer = 0;
int EndTimer = 50; 
boolean on = false,curon=true;

RF24 radio(7,8);
const uint64_t pipes[10] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL,0xF0F0F0F0E3LL,0xF0F0F0F0E4LL,0xF0F0F0F0E5LL,0xF0F0F0F0E6LL,0xF0F0F0F0E7LL,0xF0F0F0F0E8LL,0xF0F0F0F0E9LL,0xF0F0F0F0E0LL };

void setup() 
{
  Serial.begin(57600);
 
 /*
  analogWrite(LEDred, 10);
  analogWrite(LEDgreen, 0);
  analogWrite(LEDblue, 0);
 */
  printf_begin();
  radio.begin();
  radio.setRetries(15,15);
  radio.setPayloadSize(8);
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  radio.startListening();
  radio.printDetails();
}

boolean parseData(char str[10])
{
  if(str[0] == '0' || str[0] == LIGHTID)
  {
    if(str[1] == 'o' && str[2] == 'n')
    {
      Serial.println("Power ON");
      on = true;
    } else if(str[1] == 'l')
    {
      char lat[3] = {0};
      lat[0] = str[2];
      lat[1] = str[3];
      lat[2] = str[4];
      EndTimer = strtol(lat, NULL, 10);
      Serial.println("Change Latency to "+String(EndTimer));
    } else if(str[1] == 'o' && str[2] == 'f' && str[3] == 'f')
    {
      Serial.println("Power Off");
      on = false;
    } else
    {
      char red[5] = {0};
      char green[5] = {0};
      char blue[5] = {0};
      red[0] = green[0] = blue[0] = '0';
      red[1] = green[1] = blue[1] = 'X';
      red[2] = str[1];
      red[3] = str[2];
      green[2] = str[3];
      green[3] = str[4];
      blue[2] = str[5];
      blue[3] = str[6]; 
      rot = strtol(red, NULL, 16);
      gruen = strtol(green, NULL, 16);
      blau = strtol(blue, NULL, 16);
      
      Serial.println("RAW: "+String(str)+": R"+String(rot)+" G"+String(gruen)+" B"+String(blau));
    }
    return true;
  } else
  {
    //Serial.println("Befehl fuer "+String(str[0])+" - ignore");
    return false;
  }
}

void loop(){

  if ( radio.available() )
    {
      // Dump the payloads until we've gotten everything
      char gotc[10];
      bool done = false,answer=false;
      while (!done)
      {
        // Fetch the payload, and see if this was the last one.
        done = radio.read( &gotc, sizeof(char)*10 );
        
        // Spew it
        Serial.println("Empfange Befehl");
        answer = parseData(gotc);
        // Delay just a little bit to let the other unit
        // make the transition to receiver
        delay(20);
      }

      // First, stop listening so we can talk
      if(answer)
      {
        radio.stopListening();
  
        // Send the final one back.
        radio.write("ok", sizeof(char)*10 );
        Serial.println("Answer OK");
  
        // Now, resume listening so we catch the next packets.
        radio.startListening();
      }
    }
    
    if(curon != on)
    {
      if(on == false) //Abschalten
      {
        rot = 0;
        gruen = 0;
        blau = 0;
        curon = on;
      }
      else //Einschalten
      {
        if(blau == 0 && gruen == 0 && rot == 0)
        {
          //Startfarben minimum weiss
          blau = rot = gruen = 1;
        }
        curon = on;
        currot = 0;
        curgruen = 0;
        curblau = 0;
      }
        
    }
    
    if(currot != rot || curgruen != gruen || curblau != blau)
    {
      if(EndTimer == 0)
      {
        currot = rot;
        curblau = blau;
        curgruen = gruen;
      } else
      {
         if (timer <= EndTimer) timer++;
         else 
         {
           timer = 0;
           //Serial.println("IS R:"+String(currot)+" G:"+String(curgruen)+ " B:"+String(curblau));
           //Serial.println("TO R:"+String(rot)+" G:"+String(gruen)+ " B:"+String(blau));
           
           if (rot < currot) --currot;
           else if (rot > currot) ++currot;
           
           if (gruen < curgruen) --curgruen;
           else if (gruen > curgruen) ++curgruen;
           
           if (blau < curblau) --curblau;
           else if (blau > curblau) ++curblau;
         }
      }
      strip.setR(currot);
      strip.setG(curgruen);
      strip.setB(curblau);
    }
    
    

}
