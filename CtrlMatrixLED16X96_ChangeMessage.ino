//#include <SoftwareSerial.h>
#include "LedControl.h"

/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 10 is connected to the DataIn 
 pin 8 is connected to the CLK 
 pin 9 is connected to LOAD 
 ***** Please set the number of devices you have *****
 But the maximum default of 8 MAX72XX wil also work.
 */
int PIN_CLK = 2;
int PIN_CS = 3; // chip select
int PIN_DIN = 4;

/* we always wait a bit between updates of the display */
int DevNum = 24;
int Light = 3;

LedControl lc(PIN_DIN, PIN_CLK, PIN_CS, DevNum);

//int WalkSize = 7;
//int WalkPos = 0;

int WordSize = 25;
int WordPos = 0;
unsigned long oldTime = 0;
unsigned long oldWalkTime = 0;
unsigned long oldLoopTime = 0;
int loopDelay = 0;
int StartTime = 50;
int StopTime = 0;
int OffsetPos = 0;
int Pos = 0;
byte LedBuf[6][32] = {0};
int loopPos = 0;

bool StopLoop = false;

// serial control
int bufCount = 0;
char inputString[300] = {0}; //650
bool stringComplete = false;
int SetMsgSize = 0;
byte MaxMsgSize = 20;
byte MsgBuf[6][32] = {0}; 

void setup()  
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  
  for(int addr = 0; addr < DevNum; addr ++)
  {
      /*The MAX72XX is in power-saving mode on startup*/
      lc.shutdown(addr,false);
      /* Set the brightness to a medium values */
      lc.setIntensity(addr, Light);
      /* and clear the display */
      lc.clearDisplay(addr);
  }

  const byte* val1 = NULL;
  int len = 32;
  for(int i = 0; i < 6; ++ i)
  {
    val1 = lc.GetArrayByIndex(i);
    memcpy(MsgBuf[i], val1, len);
  }
}

void loop() // run over and over
{
    if(StopLoop)
      return;

    switch(loopPos) {
      case 0:
        if( WordPos == WordSize )
          ++loopPos;
        loopDelay = 0;
        ShiftLeftWord();
        break;
      case 1:
        loopDelay = 300;
        if(SignleWord())
          loopPos = 0;
        break;
    }
}

bool decideLoopTime()
{
    unsigned long currentTime = millis();
    bool hit = false;
    if( currentTime - oldLoopTime >= loopDelay )
    {
        hit = true;
        oldLoopTime = currentTime;
    }

    return hit;  
}

// if ture than exit loop
bool SignleWord()
{
    if(! decideLoopTime())
      return false;

    const byte* val1 = NULL;
    if(WordPos >= WordSize)
    {
        WordPos = 0;
        return true;
    }

    // copy 4 words into LED buffer
    int len = 32;
    for(int i = 0; i < 6; i++)
    {
        if(WordPos >= WordSize)
            WordPos = 0;
        val1 = MsgBuf[WordPos++];
        memcpy(LedBuf[i], val1, len);
    }

    drawTwoRowScreen();
    return false;
}

void ShiftLeftWord()
{
    if(! decideLoopTime())
        return;

    byte tmp[32] = {0};
    int len = 32;
    int wordOffset = 18;
    if(WordPos > WordSize)
      WordPos = 0;

//    val1 = MsgBuf[WordPos];
    const byte* val1 = NULL;
    val1 = lc.GetArrayByIndex(WordPos);
    memcpy(tmp, val1, len);

    // shift two word from memory first
     for (byte k = 0; k < 16; k += 2)
     {
      // Up-Left word with high byte
        LedBuf[0][k] <<= 1;
        LedBuf[0][k] |= ((LedBuf[0][k+1] & 0x80) >> 7);
        // Up-Left word with low byte
        LedBuf[0][k+1] <<= 1;
        // middle word with last bit
        LedBuf[0][k+1] |= ((LedBuf[1][k] & 0x80) >> 7);

        // Down-Left word with high byte
        LedBuf[0][k+16] <<= 1;
        LedBuf[0][k+16] |= ((LedBuf[0][k+16+1] & 0x80) >> 7);
        // Down-Left word with low byte
        LedBuf[0][k+16+1] <<= 1;
        // middle word with last bit
        LedBuf[0][k+16+1] |= ((LedBuf[1][k+16] & 0x80) >> 7);



        // Up-Left word with high byte
        LedBuf[1][k] <<= 1;
        LedBuf[1][k] |= ((LedBuf[1][k+1] & 0x80) >> 7);
        // Up-Left word with low byte
        LedBuf[1][k+1] <<= 1;
        // middle word with last bit
        LedBuf[1][k+1] |= ((LedBuf[2][k] & 0x80) >> 7);

         // Down-Left word with high byte
        LedBuf[1][k+16] <<= 1;
        LedBuf[1][k+16] |= ((LedBuf[1][k+16+1] & 0x80) >> 7);
        // Down-Left word with low byte
        LedBuf[1][k+16+1] <<= 1;
        // middle word with last bit
        LedBuf[1][k+16+1] |= ((LedBuf[2][k+16] & 0x80) >> 7);


        // Up-Left word with high byte
        LedBuf[2][k] <<= 1;
        LedBuf[2][k] |= ((LedBuf[2][k+1] & 0x80) >> 7);
        // Up-Left word with low byte
        LedBuf[2][k+1] <<= 1;
        // middle word with last bit
        LedBuf[2][k+1] |= ((LedBuf[3][k] & 0x80) >> 7);

         // Down-Left word with high byte
        LedBuf[2][k+16] <<= 1;
        LedBuf[2][k+16] |= ((LedBuf[2][k+16+1] & 0x80) >> 7);
        // Down-Left word with low byte
        LedBuf[2][k+16+1] <<= 1;
        // middle word with last bit
        LedBuf[2][k+16+1] |= ((LedBuf[3][k+16] & 0x80) >> 7);



        // Up-Left word with high byte
        LedBuf[3][k] <<= 1;
        LedBuf[3][k] |= ((LedBuf[3][k+1] & 0x80) >> 7);
        // Up-Left word with low byte
        LedBuf[3][k+1] <<= 1;
        // middle word with last bit
        LedBuf[3][k+1] |= ((LedBuf[4][k] & 0x80) >> 7);

         // Down-Left word with high byte
        LedBuf[3][k+16] <<= 1;
        LedBuf[3][k+16] |= ((LedBuf[3][k+16+1] & 0x80) >> 7);
        // Down-Left word with low byte
        LedBuf[3][k+16+1] <<= 1;
        // middle word with last bit
        LedBuf[3][k+16+1] |= ((LedBuf[4][k+16] & 0x80) >> 7);

        
        // Up-Left word with high byte
        LedBuf[4][k] <<= 1;
        LedBuf[4][k] |= ((LedBuf[4][k+1] & 0x80) >> 7);
        // Up-Left word with low byte
        LedBuf[4][k+1] <<= 1;
        // middle word with last bit
        LedBuf[4][k+1] |= ((LedBuf[5][k] & 0x80) >> 7);

         // Down-Left word with high byte
        LedBuf[4][k+16] <<= 1;
        LedBuf[4][k+16] |= ((LedBuf[4][k+16+1] & 0x80) >> 7);
        // Down-Left word with low byte
        LedBuf[4][k+16+1] <<= 1;
        // middle word with last bit
        LedBuf[4][k+16+1] |= ((LedBuf[5][k+16] & 0x80) >> 7);

    
        // Up-Right word with high byte
        LedBuf[5][k] <<= 1;
        LedBuf[5][k] |= ((LedBuf[5][k+1] & 0x80) >> 7);
        LedBuf[5][k+1] <<= 1;

        // Down-Right word with high byte
        LedBuf[5][k+16] <<= 1;
        LedBuf[5][k+16] |= ((LedBuf[5][k+16+1] & 0x80) >> 7);
        LedBuf[5][k+16+1] <<= 1;

        // get single word and shift 1 bit ony by one
        if(Pos <= 7) // high byte
        {
          tmp[k] <<= OffsetPos;
          tmp[k+16] <<= OffsetPos;
        }
        else //low byte
        {
          tmp[k] = tmp[k+1] << OffsetPos;
          tmp[k+16] = tmp[k+16+1] << OffsetPos;
        }

        // Up-Right word with last bit
        LedBuf[5][k+1] |= ((tmp[k] & 0x80) >> 7);
        // Down-Right word with last bit
        LedBuf[5][k+16+1] |= ((tmp[k+16] & 0x80) >> 7);
     }
    
     ++ OffsetPos;
     ++ Pos;
    
     // get next world
     if(OffsetPos >= 8)
        OffsetPos = 0;
     if(Pos >= 16)
     {
        ++ WordPos;
        Pos = 0;
     }

     drawTwoRowScreen();
}

void drawTwoRowScreen()
{
    // Turn on each row in series
    // 16x192 matrix(2x12 LED) order as below:
    // [11][10][09][08][07][06][05][04][03][02][01][00]  00:head, 15:tail
    // [23][22][21][20][19][18][17][16][15][14][13][12]
     for (byte k = 0, i = 0; k < 16; k += 2, ++ i)
     {
          lc.resetStatus();
          lc.setRow(1, i, LedBuf[5][k]);
          lc.setRow(0, i, LedBuf[5][k+1]);
          lc.setRow(13, i, LedBuf[5][k+16]);
          lc.setRow(12, i, LedBuf[5][k+16+1]);
  
          lc.setRow(3, i, LedBuf[4][k]);
          lc.setRow(2, i, LedBuf[4][k+1]);
          lc.setRow(15, i, LedBuf[4][k+16]);
          lc.setRow(14, i, LedBuf[4][k+16+1]);

          lc.setRow(5, i, LedBuf[3][k]);
          lc.setRow(4, i, LedBuf[3][k+1]);
          lc.setRow(17, i, LedBuf[3][k+16]);
          lc.setRow(16, i, LedBuf[3][k+16+1]);
  
          lc.setRow(7, i, LedBuf[2][k]);
          lc.setRow(6, i, LedBuf[2][k+1]);
          lc.setRow(19, i, LedBuf[2][k+16]);
          lc.setRow(18, i, LedBuf[2][k+16+1]);

          lc.setRow(9, i, LedBuf[1][k]);
          lc.setRow(8, i, LedBuf[1][k+1]);
          lc.setRow(21, i, LedBuf[1][k+16]);
          lc.setRow(20, i, LedBuf[1][k+16+1]);
  
          lc.setRow(11, i, LedBuf[0][k]);
          lc.setRow(10, i, LedBuf[0][k+1]);
          lc.setRow(23, i, LedBuf[0][k+16]);
          lc.setRow(22, i, LedBuf[0][k+16+1]);
          lc.action();
     }
}


void serialEvent()
{

}


