#include "TFT_eSPI.h"
#include "Free_Fonts.h" 
#include <pgmspace.h>
#include "c64n.h"
#include "c64_font.h"
#include "ntc.h"



#include "RTClib.h"

#include <WiFi.h>
#include "time.h"
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;
const char* ssid       = "CloudTest";
const char* password   = "CdAutomationTEST";


bool  RTC_EN=0;

#define MAX_IMAGE_WIDTH 478
int16_t xpos = 1;
int16_t ypos = 1;


RTC_DS1307 DS1307_RTC;

char Week_days[7][12] = {"Dom", "Lun", "Mar", "Mer", "Gio", "Ven", "Sab"};
TFT_eSPI myGLCD = TFT_eSPI();     

uint16_t CHARACTER = TFT_BLUE;
uint16_t BACKGROUND = TFT_BLUE;


static uint8_t t_int;
static uint8_t t_dec; 

void setup() {
  myGLCD.begin();
  myGLCD.invertDisplay(0);
  myGLCD.setRotation(3); 
  myGLCD.fillScreen(TFT_BLACK);
  delay(100);
  
  Serial.begin(115200);  
  Serial.println("\r\nInit.."); 
  delay(1000);
  
  delay(500);
  //Wire.begin();
  bool ot=false;
  
  myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 
  myGLCD.setTextSize(1);
  myGLCD.setFreeFont(FF18);
  myGLCD.drawString("Connessione Wifi.", 130, 155);
  ot= true;
  WiFi.begin(ssid, password);
  uint8_t retry=0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    retry++;
    Serial.print(".");
      myGLCD.fillScreen(TFT_BLACK);
      if(ot==false)
      {
        ot= true;
        myGLCD.drawString("Connessione Wifi.", 130, 155);
      }
      else
      {
        ot=false;
        myGLCD.drawString("Connessione Wifi .", 130, 155);
      }
      if(retry==20)
      {
        RTC_EN=true;
        break;
      }
  }
  if(WiFi.status() == WL_CONNECTED)
  {
    Serial.println("WIFI Connected");
  }
  else
  {
    Serial.println("WIFI KO");
  }
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  ot=false;
  while (!DS1307_RTC.begin()) {    
    if(!ot)
    {
      myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 
      myGLCD.setTextSize(1);
      myGLCD.setFreeFont(FF18);
      myGLCD.drawString("RTC ERROR", 180, 155); 
      ot=true;
    }
    //DS1307_RTC.begin();
    delay(1000);
  }
  Serial.println("RTC OK");

  //DS1307_RTC.adjust(DateTime(F(__DATE__), F(__TIME__))); //SET data e ora da software, commentare dopo la prima compilazione
  Serial.println("\r\nInit done.");
  myGLCD.fillScreen(TFT_BLACK);
  loadC64();
}

void loop() {
  static uint8_t rt=0;
  UpdateDisp();
  delay(999);
}

void loadC64()
{
  //myGLCD.pushImage(xpos, ypos,478,318, c64n);
  BACKGROUND = TFT_BLUE;
  CHARACTER = 0xA51F;//0xA2FF;//myGLCD.color16to24(myGLCD.color565(164,163,255));//myGLCD.readPixel (10,10);//RGB565 0xA2FF

  myGLCD.fillRect(1  , 1  , 478  , 318, CHARACTER);
  myGLCD.fillRect(36  , 32  , 406  , 259, BACKGROUND);

  myGLCD.setTextColor(CHARACTER,BACKGROUND); 
  myGLCD.setTextSize(0);
  myGLCD.setFreeFont(&c64_8);
  myGLCD.drawString("**** COMMODORE 64 BASIC V2 ****", 65, 39);
  myGLCD.drawString("64K RAM SYSTEM 38911 BASIC BYTES FREE", 45, 57);
  myGLCD.drawString("READY.", 37, 75);
}


bool ntc()
{
#define SIZE 50
    static double tdold;
    static bool init_b;
    static double coda[SIZE];
    static uint8_t counter;
  
    static double R1 = 9970.0;   // voltage divider resistor value
    static double Beta = 3977.0;  // Beta value
    static double To = 298.15;    // Temperature in Kelvin for 25 degree Celsius
    static double Ro = 10000.0;   // Resistance of Thermistor at 25 degree Celsius

    static double adcMax = 4095.0; // ADC resolution 12-bit (0-4095)
    static double Vs = 3.6;        // supply voltage
    if(!init_b)
    {
      init_b=true;
      for(int i=0;i<SIZE;i++)
      {
        coda[i]=20.0;
      }
    }
    
    double adc = analogRead(36);
    //adc = ADC_LUT[(int)adc];
    
    double Vout, Rt = 0;
    double T, Tc, Tf = 0;
    Vout = adc * Vs/adcMax;
    Rt = R1 * Vout / ((Vs-0.3) - Vout);
    /*
    Serial.print("V: ");
    Serial.print(Vout);
    Serial.print("V ADC: ");
    Serial.print(adc);
    Serial.print(" NTC: ");
    Serial.print(Rt);
    Serial.println(" ohm"); */
    T = 1/(1/To + log(Rt/Ro)/Beta);    // Temperature in Kelvin
    Tc = T - 273.15;                   // Celsius
    
    Tc=Tc-4.5;                         // compensazione finale
    
    coda[counter]=Tc;
    Tc=0;
    double mi=0xFFFF;
    double ma=0;
    for(int i=0;i<SIZE;i++)
    {
      if(mi>coda[i])
        mi=coda[i];
      if(ma<coda[i])
        ma=coda[i];
      Tc+=coda[i];
    }
    //Tc = Tc-mi-ma;
    T = Tc-mi;
    T = Tc-ma;
    Tc=T/(double)(SIZE-2);

    if(counter<SIZE-1)
      counter++;
    else
      counter=0;
    
    t_int = Tc;
    Tc = (Tc - (t_int));
    t_dec=Tc*10.0;
    if(t_dec>9)
      t_dec=9;  
    if(tdold!=t_dec)
    {
      tdold=t_dec;
      return true;
    }
    return false;
}


void UpdateDisp(){
   static bool blink;
   static int c1=255;
   static int c2=255;
   static int c3=255;
   static int c4=255;
   static uint8_t day_old;
   static bool connection;

   
  int h; 
  int e;   
  uint8_t s;
  uint8_t d_w;
  uint8_t m,d;
  uint16_t y;

  int d1;  
  int d2;  
  int d3;  
  int d4;  
  
if(RTC_EN==1)
{
  DateTime now = DS1307_RTC.now();
  h = now.hour();
  e = now.minute();
  s=now.second();
  m=now.month();
  d=now.day();
  y=now.year();
  d_w=now.dayOfTheWeek();
  if(!connection)
  {
    connection=true;
    myGLCD.setTextColor(CHARACTER,BACKGROUND); 
    myGLCD.setTextSize(0);
    myGLCD.setFreeFont(&c64_8);
    myGLCD.fillRect(37  , 75  , 100  , 10, TFT_RED);
    myGLCD.drawString("READY RTC MODE.", 37, 75);
  }
}else
{  
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("non riesco a connettermi al server");
    return;
  }
  //Serial.println(&timeinfo, "%a, %B %d %Y # ");
  char t[5]; 
  strftime(t, 3, "%H", &timeinfo);
  h = atoi(t);
  memset(t, 0, sizeof t);
  strftime(t, 3, "%M", &timeinfo);
  e = atoi(t);
  memset(t, 0, sizeof t);
  strftime(t, 3, "%S", &timeinfo);
  s = atoi(t);
  memset(t, 0, sizeof t);
  strftime(t, 3, "%d", &timeinfo);
  d = atoi(t);
  memset(t, 0, sizeof t);
  strftime(t, 3, "%m", &timeinfo);
  m = atoi(t);
  memset(t, 0, sizeof t);
  strftime(t, 5, "%Y", &timeinfo);
  y = atoi(t);
  memset(t, 0, sizeof t);
  strftime(t, 3, "%Ou", &timeinfo);
  d_w = atoi(t);
  if(!connection)
  {
    connection=true;
    myGLCD.setTextColor(CHARACTER,BACKGROUND); 
    myGLCD.setTextSize(0);
    myGLCD.setFreeFont(&c64_8);
    myGLCD.fillRect(37  , 75  , 100  , 10, TFT_RED);
    myGLCD.drawString("READY NTP MODE.", 37, 75);
    DS1307_RTC.adjust(DateTime(y,m,d,h,e,s));
  }
  /*
  Serial.print("# ");
  Serial.print(t);
  Serial.print(" #");
  Serial.print(d);
  Serial.print("/");
  Serial.print(m);
  Serial.print("/");
  Serial.print(y);
  Serial.print("-");
  Serial.println(d_w);*/
}

  //loadC64();
  if ((h >= 10) && (h < 20)) {     
    d1 = 1; 
    d2 = h - 10;  
  } else if ( (h >= 20) && (h <= 24)) {    
    d1 = 2; 
    d2 = h - 20;   
  }
  else
  {
    d1 = 0; 
    d2 = h;
  }
    

  if ((e >= 10)) 
  {  
    d3 = e/10 ; 
   d4 = e - (d3*10);  
  } 
  else 
  {
    d3 = 0;
    d4 = e;
  }  
 
  myGLCD.setTextColor(CHARACTER,BACKGROUND); 
  myGLCD.setTextSize(4);
  //myGLCD.setFreeFont(FF20);
  myGLCD.setFreeFont(&c64f_20);
  
  uint16_t start_d = 53;
  uint16_t start_y = 115;
  
// First Digit
//if(d1 != c1){ 
if(d4 != c4){  
  myGLCD.fillRect(start_d  , start_y-1 , 381  , 110, BACKGROUND);
  if(d1==1)
    myGLCD.drawNumber(d1,start_d+20,start_y); 
  else
    myGLCD.drawNumber(d1,start_d,start_y);


  if(d2==1)
    myGLCD.drawNumber(d2,start_d+110,start_y); 
  else
    myGLCD.drawNumber(d2,start_d+90,start_y); 

  if(d3==1)
    myGLCD.drawNumber(d3,start_d+220,start_y);  
  else
    myGLCD.drawNumber(d3,start_d+200,start_y);

  if(d4==1)
    myGLCD.drawNumber(d4,start_d+310,start_y); 
  else
    myGLCD.drawNumber(d4,start_d+290,start_y); 
}

  myGLCD.setTextColor(CHARACTER,BACKGROUND); 
  myGLCD.setTextSize(0);
  myGLCD.setFreeFont(&c64f_16);
  
  //s=44;
  if(s<10)
  {
    myGLCD.fillRect(start_d+175  , start_y-11 , 29  , 19, BACKGROUND);
    myGLCD.drawString("0", start_d+175, start_y-10); 
    myGLCD.drawNumber(s,start_d+189,start_y-10);
  }
  else
  {
    myGLCD.fillRect(start_d+175  , start_y-11 , 29  , 19, BACKGROUND);
    if(s<20)
      myGLCD.drawNumber(s,start_d+178,start_y-10);
    else
      myGLCD.drawNumber(s,start_d+175,start_y-10);
  }

  uint16_t start_date =  start_d+20; 
  uint16_t start_date_y = 235;
  
  if(day_old!=d)
  {  
    myGLCD.fillRect(start_d  , start_date_y , 381  , 22, BACKGROUND);
    myGLCD.setTextColor(CHARACTER,BACKGROUND); 
    myGLCD.setTextSize(0);
    myGLCD.setFreeFont(&c64f_14);
    
    myGLCD.drawString(Week_days[d_w], start_d, start_date_y); 
    if(d<10)
    {
      myGLCD.drawString("0", start_date+50, start_date_y);
      myGLCD.drawNumber(d,start_date+70,start_date_y);
    }
    else
    {
      myGLCD.drawNumber(d,start_date+50,start_date_y);
    }
    myGLCD.drawString("/", start_date+92, start_date_y);
    
    if(m<10)
    {
      myGLCD.drawString("0", start_date+115, start_date_y);
      myGLCD.drawNumber(m,start_date+135,start_date_y);
    }
    else
    {
      myGLCD.drawNumber(m,start_date+115,start_date_y);
    }
    myGLCD.drawString("/", start_date+160, start_date_y);
    myGLCD.drawNumber(y,start_date+183,start_date_y);
    day_old=d;
  }
  
  if(ntc())
  {
    char str[5];
    sprintf(str,"%d.%d",t_int,t_dec);
    myGLCD.setTextColor(CHARACTER,BACKGROUND); 
    myGLCD.setTextSize(0);
    myGLCD.setFreeFont(&c64f_14);
  
    myGLCD.fillRect(start_date+273  , start_date_y-2 , 65  , 25, BACKGROUND);
    myGLCD.drawString(str, start_date+275, start_date_y);
    
    myGLCD.fillCircle(start_d+363, start_date_y+2, 4,CHARACTER);
    myGLCD.fillCircle(start_d+363, start_date_y+2, 2,BACKGROUND);

    myGLCD.setTextColor(CHARACTER,BACKGROUND); 
    myGLCD.setTextSize(0);
    myGLCD.setFreeFont(&c64_8);
    myGLCD.drawString("C", start_d+361, start_date_y+8);
  }

  
  if(!blink)
  {
    myGLCD.fillRect(37  , 87  , 9  , 8, BACKGROUND);
    myGLCD.fillCircle(start_d+187, start_y+37, 4,BACKGROUND);
    myGLCD.fillCircle(start_d+187, start_y+57, 4,BACKGROUND);
    blink=true;
  }
  else
  {
    myGLCD.fillRect(37  , 87  , 9  , 8, CHARACTER);
    myGLCD.fillCircle(start_d+187, start_y+37, 4,CHARACTER);
    myGLCD.fillCircle(start_d+187, start_y+57, 4,CHARACTER);
    blink=false;
  }

  c1 = d1;
  c2 = d2;
  c3 = d3;
  c4 = d4;

}
