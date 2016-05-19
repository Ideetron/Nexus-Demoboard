/******************************************************************************************
* Copyright 2015, 2016 Ideetron B.V.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************************/
/****************************************************************************************
* File:     Nexus_Demoboard_V10.ino
* Author:   Gerben den Hartog
* Compagny: Ideetron B.V.
* Website:  http://www.ideetron.nl/LoRa
* E-mail:   info@ideetron.nl
****************************************************************************************/
/****************************************************************************************
* Created on:         15-01-2016
* Supported Hardware:  ID150170-01 with ID150119-02 with a RFM95
* 
* Description
* 
* Firmware version: 1.0
* First version
****************************************************************************************/

/*
*****************************************************************************************
* INCLUDE FILES
*****************************************************************************************
*/
#include <SPI.h>
#include "AES-128_V10.h"
#include "Encrypt_V30.h"
#include "Nexus_Demoboard_V10.h"
#include "RFM95_V21.h"
#include "LoRaMAC_V11.h"
#include "Waitloop_V11.h"

/*
*****************************************************************************************
* GLOBAL VARIABLES
*****************************************************************************************
*/
unsigned char NwkSkey[16] = {
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
  0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};

unsigned char AppSkey[16] = {
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
  0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};

unsigned char DevAddr[4] = {
  0x02, 0x01, 0x05, 0x04
};

void setup() 
{
  //Switch all analog pins to digital
  DIDR0 = 0x00;
    
   //Initialize the UART
  Serial.begin(9600);

   //Initialise the SPI port
  SPI.begin();
  SPI.beginTransaction(SPISettings(4000000,MSBFIRST,SPI_MODE0));

  //Initialize I/O pins  
  pinMode(MOV,INPUT);

  //Set led pins to input pullup
  pinMode(LED1,INPUT);
  digitalWrite(LED1,HIGH);
  pinMode(LED2,INPUT);
  digitalWrite(LED2,HIGH);
  
  pinMode(PIR,INPUT);
  
  pinMode(DS2401,OUTPUT);
  digitalWrite(DS2401,HIGH);
  
  pinMode(MFP,INPUT);
  pinMode(DIO0,INPUT);
  pinMode(DIO1,INPUT); 
  pinMode(DIO5,INPUT);
  pinMode(DIO2,INPUT);
  pinMode(CS,OUTPUT);
  digitalWrite(CS,HIGH);
  
  pinMode(LED_N,OUTPUT);

  WaitLoop_Init();

  //Wait until RFM module is started
  WaitLoop(20);

  //Clear timer 1
  TIFR1 = 0x02;
  TCNT1 = 0x0000;
}

void loop() 
{
  unsigned char i;
  
  unsigned char LDR_Value;
  unsigned char AR_Value;

  unsigned char LED_Status = 0x00;
  unsigned char Button_Status = 0x00;

  unsigned int Message_Interval = 600; //times 0.1 second
  unsigned int Interval_Counter = 0x0000;

  unsigned char Data_Tx[256];
  unsigned char Data_Rx[64];
  unsigned char Data_Length_Tx;
  unsigned char Data_Length_Rx = 0x00;

  //Initialize RFM module
  RFM_Init();

  digitalWrite(LED_N,HIGH);

  while(1)
  {
    if((TIFR1 & 0x02) == 0x02)
    {
      //Raise interval counter
      Interval_Counter++;
      //Clear interrupt;
      TIFR1 = 0x02;
      //Reset timer
      TCNT1 = 0x0000;

      //Check pushbuttons
      Button_Status = Check_Buttons();
      Set_LEDs(LED_Status);

      //Force message when button is pressed
      if(Button_Status != 0x00)
      {
        Interval_Counter = Message_Interval; 
      }      
    }

    if(Interval_Counter == Message_Interval)
    {
      Interval_Counter = 0x0000; 
      
      //Read LDR sensor
      LDR_Value = analogRead(LDR);
      
      //Read Adjustable resistor
      AR_Value = analogRead(RES);

      //Concstruct data
      Data_Tx[0] = Button_Status;
      Data_Tx[1] = LDR_Value;
      Data_Tx[2] = AR_Value;
      Data_Length_Tx = 0x03;
                 
      Data_Length_Rx = LORA_Cycle(Data_Tx, Data_Rx, Data_Length_Tx);
    }    

    if(Data_Length_Rx != 0x00)
    {
      for(i = 0; i < Data_Length_Rx; i++)
      {
        Serial.write(Data_Rx[i]);
      }

      LED_Status = Data_Rx[0];

      Set_LEDs(LED_Status);      

      Data_Length_Rx = 0x00;
    }
  }//While(1)
}

unsigned char Check_Buttons()
{
  unsigned char Button_Status = 0x00;

  pinMode(LED1,INPUT);
  digitalWrite(LED1,HIGH);
  pinMode(LED2,INPUT);
  digitalWrite(LED2,HIGH);

  if(digitalRead(LED1) == LOW)
  {
    Button_Status = Button_Status + 0x01;
  }

  if(digitalRead(LED2) == LOW)
  {
    Button_Status = Button_Status + 0x02;
  }
  
  return Button_Status;
}

void Set_LEDs(unsigned char LED_Status)
{
    //Switch LEDs back on
  if((LED_Status & 0x01) == 0x01)
  {
    pinMode(LED1,OUTPUT);
    digitalWrite(LED1,LOW);
  }
  else
  {
    pinMode(LED1,INPUT);
    digitalWrite(LED1,HIGH);
  }

  if((LED_Status & 0x02) == 0x02)
  {
    pinMode(LED2,OUTPUT);
    digitalWrite(LED2,LOW);    
  }
  else
  {
    pinMode(LED2,INPUT);
    digitalWrite(LED2,HIGH);
  }
}
