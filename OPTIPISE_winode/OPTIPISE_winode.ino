// entrées sorties
#include <stdio.h>

// temperature et humidite
#include <DHT22.h>

// temperature IR
#include <i2cmaster.h> 

// pression
#include <Wire.h>
#include <Adafruit_BMP085.h>

// fichiers
#include "ir.c"
#include "lum.c"
#include "presence.c"

// radio
#include <RF12.h>
#include <Ports.h>

// microSD
#include <SD.h>

// pression
Adafruit_BMP085 bmp;

// temperature et humidite
#define DHT22_PIN 7
DHT22 myDHT22(DHT22_PIN); 

// presence
#define PIN_PRES 3


// microSD
//Sd2Card card;
//SdVolume volume;
//SdFile root;
const int chipSelect = 4; 
File dataFile;
String dataString;

// radio
#define myNodeID 10          //node ID of tx (range 0-30)
#define network     210      //network group (can be in the range 1-250).
#define freq RF12_868MHZ     //Freq of RF12B can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. Match freq to module
#define RADIOLED 6

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

#define SERIAL 1 // valide ou devalide la liaison serie
#define RADIO 1 // valide ou devalide la liaison radio
#define WRITE_SD 0
#define READ_SD 0
#define TEMPO 2000 // attention: 2s minimum (pour DHT22) > 1000 minimum
#define IR 1
#define TH 1
#define LUM 1
#define PRESENCE 1
#define PRESSION 1

// nombre de données
int N = IR + 2*TH + LUM + PRESENCE + PRESSION ; 

int tempIR;
int tempdecIR;
int temp;
int tempdec;
int hum;
int humdec;
int pressi;
int pressidec;
float DHTtemperature;
float DHThumidite;
float IRtemperature;
float BMPpression;

void setup(void)
{
// start serial port
#if SERIAL==1
  Serial.begin(9600);
#endif
  Serial.println("--------------- INITIALISATION ---------------");
  
// temperature IR
#if IR==1
ir_setup();
#endif

// luminosite
#if LUM==1
lum_setup();
#endif

// pression
#if PRESSION==1
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) {}
  }
#endif

// presence
#if PRESENCE==1
presence_setup(); // interruption 1 enable
#endif

delay(TEMPO);

// radio
#if RADIO==1
  rf12_initialize(myNodeID,freq,network);   //Initialize RFM12 with settings defined above  

 Serial.println("RFM12B Transmitter");

 Serial.print("Node: "); 
 Serial.print(myNodeID); 
 Serial.print(" Freq: "); 
 if (freq == RF12_433MHZ) Serial.print("433Mhz");
 if (freq == RF12_868MHZ) Serial.print("868Mhz");
 if (freq == RF12_915MHZ) Serial.print("915Mhz"); 
 Serial.print(" Network: "); 
 Serial.println(network);
 Serial.println();
 pinMode(RADIOLED, OUTPUT);
 digitalWrite(RADIOLED, HIGH);
delay(TEMPO);
#endif

// write SD
#if WRITE_SD==1 

  Serial.print("Initializing SD card...");
 
//  pinMode(10, OUTPUT);

  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card is inserted?");
    Serial.println("* Is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    return;
  } else {
   Serial.println("Wiring is correct and a card is present."); 
  }

  // verifier creation/lecture du fichier
  Serial.println("Creating datalog.txt...");
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  dataFile.println("data OPTIPISE");

  if (SD.exists("datalog.txt")) {
    Serial.println("datalog.txt exists.");
  } else {
    Serial.println("datalog.txt doesn't exist.");
  }

  dataFile.close();  
  
  Serial.println();

  delay(TEMPO);
#endif
}

void loop(void)
{ 
Serial.println("--------------- CAPTEURS ---------------");
  
 digitalWrite(RADIOLED, HIGH); //Green LED flashes before gathering data from sensors 
 delay (10);
 digitalWrite(RADIOLED, LOW);
 
  
// temperature et humidite
#if TH==1
  DHT22_ERROR_t errorCode;

  errorCode = myDHT22.readData();
  
  switch(errorCode)
  {
    case DHT_ERROR_NONE:
      Serial.print("DHT Data ");
      Serial.print(myDHT22.getTemperatureC());
      Serial.print("C ");
      Serial.print(myDHT22.getHumidity());
      Serial.print("% ");
	  
      //char buf[128];
      //sprintf(buf, "Integer-only reading: Temperature %hi.%01hi C, Humidity %i.%01i %% RH",
      //             myDHT22.getTemperatureCInt()/10, abs(myDHT22.getTemperatureCInt()%10),
      //             myDHT22.getHumidityInt()/10, myDHT22.getHumidityInt()%10);
      //Serial.print(buf);
      break;
    case DHT_ERROR_CHECKSUM:
      Serial.print("check sum error ");
      Serial.print(myDHT22.getTemperatureC());
      Serial.print("°C ");
      Serial.print(myDHT22.getHumidity());
      Serial.print("%");
      break;
    case DHT_BUS_HUNG:
      Serial.println("BUS Hung ");
      break;
    case DHT_ERROR_NOT_PRESENT:
      Serial.println("Not Present ");
      break;
    case DHT_ERROR_ACK_TOO_LONG:
      Serial.println("ACK time out ");
      break;
    case DHT_ERROR_SYNC_TIMEOUT:
      Serial.println("Sync Timeout ");
      break;
    case DHT_ERROR_DATA_TIMEOUT:
      Serial.println("Data Timeout ");
      break;
    case DHT_ERROR_TOOQUICK:
      Serial.println("Polled to quick ");
      break;
  }
  delay(TEMPO);
#endif
  
// Luminosite  
#if LUM==1
  lum_loop();
  
  Serial.print("luminosite = ");
  Serial.print(lum.photocellReading); // the raw analog reading

   if (lum.photocellReading < 10) {
   Serial.println(" - Dark");
   } else if (lum.photocellReading < 200) {
   Serial.println(" - Dim");
   } else if (lum.photocellReading < 500) {
   Serial.println(" - Light");
   } else if (lum.photocellReading < 800) {
   Serial.println(" - Bright");
   } else {
   Serial.println(" - Very bright");
   }
   delay(TEMPO);
#endif

// temperature IR
#if IR==1
  ir_loop();
  Serial.print("temperature IR = ");
  Serial.println(ir.celcius);
  delay(TEMPO);
#endif
    
// pression
#if PRESSION==1
    Serial.print("Temperature = ");
    Serial.print(bmp.readTemperature());
    Serial.println(" °C");
    
    Serial.print("Pressure = ");
    Serial.print(bmp.readPressure());
    Serial.println(" Pa");
    
    Serial.print("Altitude = ");
    Serial.print(bmp.readAltitude());
    Serial.println(" meters");

    Serial.print("Real altitude = ");
    Serial.print(bmp.readAltitude(101500));
    Serial.println(" meters");
    
    delay(TEMPO);
#endif

// presence (interruption 1)
#if PRESENCE==1
  if(validPres == 1){ // was motion detected
    Serial.println("Motion Detected");
    } 
  else {
    Serial.println ("No Motion");
    }
  delay(TEMPO);
#endif
    
    
Serial.println(); 

#if IR==1  
 IRtemperature = ir.celcius;
 emontx.celciusIR = int(IRtemperature*100); 
#endif
#if LUM==1
 emontx.luminosite = lum.photocellReading;
#endif
#if PRESENCE==1
 emontx.presence = validPres;
 validPres = 0; 
#endif
#if TH==1
 DHTtemperature = myDHT22.getTemperatureC();
 DHThumidite = myDHT22.getHumidity();
 emontx.temperature = int(DHTtemperature*100);
 emontx.humidite = int(DHThumidite*100);
#endif
#if PRESSION==1
BMPpression = bmp.readPressure();
emontx.pression = int(BMPpression/10);

#endif


 digitalWrite(RADIOLED, HIGH); //Green LED flashes after gathering data from sensors 
 delay (10);
digitalWrite(RADIOLED, LOW);
delay(TEMPO);

// ecriture microSD
#if WRITE_SD==1
  Serial.println("--------------- MICRO SD ---------------");

#if LUM==1
  reccord_sd(emontx.luminosite);
#endif

#if IR==1
  tempIR = emontx.celciusIR/100;
  reccord_sd(tempIR);
  
  tempdecIR = emontx.celciusIR - tempIR*100;
  reccord_sd(tempdecIR);
#endif

#if PRESENCE==1
  reccord_sd(emontx.presence);
#endif

#if TH==1
  temp = (int)DHTtemperature;
  reccord_sd(temp);

  tempdec = (DHTtemperature)*100 - temp*100;
  reccord_sd(tempdec);

  hum = (int)DHThumidite;
  reccord_sd(hum);
  
  humdec = (DHThumidite)*100 - hum*100;
  reccord_sd(humdec);
#endif

#if PRESSION==1
  pressi = int(BMPpression/100);
  reccord_sd(pressi);
  
  pressidec = int(BMPpression)-pressi;
  reccord_sd(pressidec);
#endif


  Serial.println();

#endif

// lecture micro SD
#if READ_SD==1
  myFile = SD.open("test.txt", FILE_WRITE);
  
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("testing 1, 2, 3.");
	// close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
  
  // re-open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("test.txt:");
    
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
    	Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
  	// if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
#endif

// radio 
#if RADIO==1
Serial.println("--------------- RADIO ---------------");
  
  int i = 0; 
  while (!rf12_canSend() && i<10) {
    rf12_recvDone(); 
    i++;}
  rf12_sendStart(0, &emontx, sizeof emontx);

#if IR==1
Serial.print("temperature IR*100: ");Serial.println(emontx.celciusIR);
#endif
#if LUM==1
Serial.print("luminosite: ");Serial.println(emontx.luminosite);
#endif
#if PRESENCE==1
Serial.print("presence: ");Serial.println(emontx.presence);
#endif
#if TH==1
Serial.print("temperature*100: ");Serial.println(emontx.temperature);
Serial.print("humidite*100: ");Serial.println(emontx.humidite);
#endif
#if PRESSION==1
Serial.print("pression*10 : ");Serial.println(emontx.pression);
#endif

Serial.println();

//LED blinks twice when sending data with RFM12B 
digitalWrite(RADIOLED, HIGH);
delay(50);
digitalWrite(RADIOLED, LOW);
delay(50);
digitalWrite(RADIOLED, HIGH);
delay(50);
digitalWrite(RADIOLED, LOW);

  delay(TEMPO);
  
#endif
}


#if WRITE_SD==1
void reccord_sd(int capteur)
{
  dataString = "";

  dataString += String(capteur);  
  dataString += " ; ";
  
  dataFile = SD.open("datalog.txt", FILE_WRITE);

  if (dataFile) {
    dataFile.println(dataString);
    Serial.println(dataString);    
    dataFile.close();
  } else {
    Serial.println("error opening datalog.txt");
  }
  
  delay(TEMPO);
}
#endif
