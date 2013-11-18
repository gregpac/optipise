#include <stdio.h>

// radio
#include <JeeLib.h>

// Affichage EmonGLCD
#include <GLCD_ST7565.h>
#include <JeeLib.h>
#include <avr/pgmspace.h>
#include "utility/font_clR6x8.h"

// radio
#define myNodeID 30          //node ID of Rx (range 0-30) 
#define network     210      //network group (can be in the range 1-250).
#define freq RF12_868MHZ     //Freq of RF12B can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. Match freq to module
#define GREENLED 6
typedef struct 
{ 
  int celciusIR;  
  int luminosite;
  int presence;
  int temperature;
  int humidite;
  int pression;
} PayloadTX ;      // create structure - a neat way of packaging data for RF comms

PayloadTX emontx;  

const int emonTx_NodeID=10;            //emonTx node ID

// affichage EmonGLCD
GLCD_ST7565 glcd;

char chaine1[22];
char chaine2[22];
char chaine3[22];
char chaine4[22];
char chaine5[22];
char chaine6[22];
char chaine7[22];
char chaine8[22];

int tempIR;
int tempdecIR;
int temp;
int tempdec;
int hum;
int humdec;
int pressi;
int pressidec;

#define IR 1
#define TH 1
#define LUM 1
#define PRESENCE 1
#define PRESSION 1


void setup(void)
{
// radio
  rf12_initialize(myNodeID,freq,network);   //Initialize RFM12 with settings defined above  
  Serial.begin(9600); 
  Serial.println("RF12B demo Receiver - Simple demo"); 
  
 Serial.print("Node: "); 
 Serial.print(myNodeID); 
 Serial.print(" Freq: "); 
 if (freq == RF12_433MHZ) Serial.print("433Mhz");
 if (freq == RF12_868MHZ) Serial.print("868Mhz");
 if (freq == RF12_915MHZ) Serial.print("915Mhz"); 
 Serial.print(" Network: "); 
 Serial.println(network);
 
 pinMode(GREENLED, OUTPUT);
 digitalWrite(GREENLED, LOW);
 
// Affichage EmonGLCD
//    rf12_initialize(1, RF12_868MHZ);
//    rf12_sleep(RF12_SLEEP);

    glcd.begin();
    glcd.backLight(255);
    glcd.setFont(font_clR6x8);    
    
    glcd.drawCircle(5, 5, 5, WHITE);

    glcd.refresh();
}

void loop(void)
{ 
// radio
 if (rf12_recvDone()){    
  if (rf12_crc == 0 && (rf12_hdr & RF12_HDR_CTL) == 0) {
    
    int node_id = (rf12_hdr & 0x1F);		  //extract nodeID from payload
        
    if (node_id == emonTx_NodeID)  {             //check data is coming from node with the corrct ID
        emontx=*(PayloadTX*) rf12_data;            // Extract the data from the payload 

digitalWrite(GREENLED, HIGH);
delay(50);
digitalWrite(GREENLED, LOW);
delay(50);
digitalWrite(GREENLED, HIGH);
delay(50);
digitalWrite(GREENLED, LOW);


#if IR==1
Serial.print("temperature IR: ");Serial.println(emontx.celciusIR/100);
#endif
#if LUM==1
Serial.print("luminosite: ");Serial.println(emontx.luminosite);
#endif
#if PRESENCE==1
Serial.print("presence: ");Serial.println(emontx.presence);
#endif
#if TH==1
Serial.print("temperature: ");Serial.println(emontx.temperature/100);
Serial.print("humidite: ");Serial.println(emontx.humidite/100);
#endif
#if PRESSION==1
Serial.print("pression HPa: ");Serial.println(emontx.pression/10);
#endif

Serial.println();
  }
 }
}
sprintf(chaine1, "OPTIPISE radiocheck");
sprintf(chaine2, "---------------------");

// Affichage EmonGLCD
#if IR==1
  if (emontx.celciusIR<-50)
  {sprintf(chaine3, "IR : erreur");}
  else
 { tempIR=int(emontx.celciusIR/100);
  tempdecIR=int(emontx.celciusIR)-tempIR*100;
  sprintf(chaine3, "IR: %d,%2d", tempIR, tempdecIR);}
#endif
#if LUM==1
  sprintf(chaine4, "luminosite: %d", emontx.luminosite);
#endif
#if PRESENCE==1
  sprintf(chaine5, "presence: %d", emontx.presence);
#endif
#if TH==1
  if (emontx.temperature < -70)
  {sprintf(chaine6, "temperature : erreur");}
  else
  {temp = int(emontx.temperature/100);
  tempdec = (emontx.temperature) - temp*100;
  sprintf(chaine6, "temperature: %d,%2d", temp, tempdec);}
 
  if (emontx.humidite<=0)
  {sprintf(chaine7, "humidite : erreur");}
  else
  {hum = int(emontx.humidite/100);
  humdec = (emontx.humidite) - hum*100;
  sprintf(chaine7, "humidite: %d,%2d", hum, humdec);}
#endif
#if PRESSION==1
  pressi = int(emontx.pression/10);
  pressidec = int(emontx.pression) - pressi*10;
  sprintf(chaine8, "pression: %d,%2d", pressi, pressidec);
#endif

            
    // draw a string at a location, use _p variant to reduce RAM use

  glcd.drawString(0,  0, chaine1);
  glcd.drawString(0,  8, chaine2);

#if IR==1
  glcd.drawString(0,  16, chaine3);
#endif
#if LUM==1
  glcd.drawString(0,  24, chaine4);
#endif
#if PRESENCE==1
  glcd.drawString(0,  32, chaine5);
#endif
#if TH==1
  glcd.drawString(0,  40,  chaine6);
  glcd.drawString(0,  48, chaine7);
#endif
#if PRESSION==1
  glcd.drawString(0,  56, chaine8);
#endif


  glcd.refresh();
  glcd.clear();
    
//    Sleepy::powerDown(); // power consumption is now only the GLCD + backlight

}
