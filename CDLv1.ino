/*
 * Cheap Data Logger (CDL) V1
 * 
 * Use a ILI9341 240 x 320 display to show temperature collected with 
 * Adafruit MAX31856 Universal Thermocouple Amplifier (K, J, N, R, S, T, E, or B type thermocouple)
 * Actually using Arduino Mega 2560
 * 
 * For ILI connections info here
 * http://bobdavis321.blogspot.fr/2015/07/22-or-24-or-28-inch-spi-tft-lcd-ili9341.html
 * 
 * Here MAX info here
 * https://learn.adafruit.com/adafruit-max31856-thermocouple-amplifier/overview
 * 
 * Bug in Adafruit MAX31856 library
 * Solution here, thanks to jr226retail
 * 
 * https://forums.adafruit.com/viewtopic.php?f=19&t=104926&p=535630#p535630
 * 
 * 2 simple 10 kOhm potentiometers are used as controls to select the screen and the thermocouple.
 * In the present demo only one thermocouple is implemented: the CDL is easily modifiable
 * to show up to 7 different sensors (search this file for 
 * "//Simulated temperature, change to your sensor"
 * 
 */


//Available colors

#define Black           0x0000      /*   0,   0,   0 */
#define Navy            0x000F      /*   0,   0, 128 */
#define DarkGreen       0x03E0      /*   0, 128,   0 */
#define DarkCyan        0x03EF      /*   0, 128, 128 */
#define Maroon          0x7800      /* 128,   0,   0 */
#define Purple          0x780F      /* 128,   0, 128 */
#define Olive           0x7BE0      /* 128, 128,   0 */
#define LightGrey       0xC618      /* 192, 192, 192 */
#define DarkGrey        0x7BEF      /* 128, 128, 128 */
#define Blue            0x001F      /*   0,   0, 255 */
#define Green           0x07E0      /*   0, 255,   0 */
#define Cyan            0x07FF      /*   0, 255, 255 */
#define Red             0xF800      /* 255,   0,   0 */
#define Magenta         0xF81F      /* 255,   0, 255 */
#define Yellow          0xFFE0      /* 255, 255,   0 */
#define White           0xFFFF      /* 255, 255, 255 */
#define Orange          0xFD20      /* 255, 165,   0 */
#define GreenYellow     0xAFE5      /* 173, 255,  47 */
#define Pink            0xF81F

// Colors selection for the 8 lines in Screen 1 and the 7 graphs point in Screen 2
int Colors[] = {Red, Blue, Yellow, Pink, Green, Cyan, Orange, Orange};

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Adafruit_MAX31856.h>
#include <TimeLib.h>

#define TFT_DC 47 //DC for the ILI9341, 1kΩ / 1kΩ voltage divider to switch from 5 to 3.3 V ILI inputs
#define TFT_CS 48 //CS for the ILI9341, 1kΩ / 1kΩ voltage divider to switch from 5 to 3.3 V ILI inputs
#define MAX_CS 53 //CS for the MAX31856, direct connection

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_MAX31856 max = Adafruit_MAX31856(MAX_CS);

#define control1Pin A0 //Potentiometer Switch Screen 1 (all T) to Graph
#define control2Pin A1 //Potentiometer Select to select wich T to show

#define OFFSET 35       //Offset for first column text allignment on screen
#define OFFSETX 160     //Offset for second column text allignment on screen
#define Range1 620      //Potentiometer Selection ranges for screen selection
#define Size 288        //Data to store in memory and display
#define DeltaTime 1     //Seconds to wait to store a temperature

int line=0;
int PreviousTime;
int Control1;
int Control2;
int Screen;
int ScreenPrev;
char outstr[6];
float Stat[3];
int T[Size][7];          //Saved data for 7 Sensors
int Selection;

void setup() {
  Serial.begin(9600);
  //Start ILI9341 screen and set rotation
  tft.begin();
  tft.setRotation(3);
  //Start MAX31856 and set thermocouple type
  max.begin();
  max.setThermocoupleType(MAX31856_TCTYPE_K);

  //Fill T[][] of 20 C
  for (int l=0; l<=7; l++) {for (int i=0; i<Size; i++) {
    T[i][l]=2300;
  }
  }
  
  //Screen borders
  tft.setTextColor(Blue, White);  
  tft.setTextSize(3);
  tft.drawRect(0, 0, 320-1, 240-1, Blue);
  tft.drawRect(1, 1, 320-3, 240-3, Red);
  
}

void loop() {

  //Read MAX temperature
  //In this demo only 1 available, the others (sensors) temperatures are just simulated
  float temperature = max.readThermocoupleTemperature();
  Serial.print(temperature);
  //Read actual time
  time_t TimeNow = now();
  //Read control1 to select the screen
  Control1 = analogRead(control1Pin);
  Control2 = analogRead(control2Pin);
  Serial.print(" C1 ");
  Serial.print(Control1);
  Serial.print(" C2 ");
  Serial.print(Control2);
  Serial.print(" ");

  //Read actual selection and save it
  for(int i=0; i<=7; i++) {
    if (Control2 >= i*128 && Control2 < i*128+128) {
      Selection = i;
    }
  }

  if (Control1<=Range1) {
  Screen = 1;
  }
  
  if (Control1 > Range1 && Control2 < 7*128) {
    Screen = 2;
  } 
  
  if (Control1 > Range1 && Control2 >= 7*128) {
    Screen = 3;
  }
  
  if (ScreenPrev != Screen) {
    //New screen, erase previous
    tft.drawRect(2, 2, 320-5, 240-5, Black);
    tft.fillRect(2, 2, 320-5, 240-5, Black);
  }

  Serial.print("Screen Selected : ");
  Serial.print(Screen);
  Serial.print(" Selection : ");
  Serial.print(Selection);
  
  switch (Screen) {
    case 1:
    {
    //Screen1, shows all temperatures and time
    
    //Cancel pointer position
    tft.setCursor(5, 3);
    tft.print(" ");
    for (int i=0; i<=6; i++) {
      tft.setCursor(5, i*30+30);
      tft.print(" ");
    }

    //Draw pointer
    tft.setTextColor(White, Black);
    if (Control2<128) {
      tft.setCursor(5, 3);
      tft.print(F(">"));
    } else { for(int i=1; i<=7; i++) {
      if (Control2 > i*128 && Control2 < i*128+128) {
        tft.setCursor(5, 30*i);
        tft.print(F(">"));
    }
  }
  }

  //Use dtostrf to write the temperature with the right format into outstr
  dtostrf(temperature,4, 1, outstr);
  tft.setTextSize(3);
  tft.setTextColor(Colors[0], Black);
  tft.setCursor(OFFSET, 3);
  tft.print(F("T01 "));
  tft.setCursor(OFFSET+OFFSETX, 3);
  tft.print(outstr);

  //Simulated temperature, change to your sensor
  dtostrf(temperature+10,4, 1, outstr);
  
  tft.setTextColor(Colors[1], Black);
  tft.setCursor(OFFSET, 30);
  tft.print(F("T02 "));
  tft.setCursor(OFFSET+OFFSETX, 30);
  tft.print(outstr);

  //Simulated temperature, change to your sensor
  dtostrf(temperature+20,4, 1, outstr);

  tft.setTextColor(Colors[2], Black);
  tft.setCursor(OFFSET, 60);
  tft.print(F("T03 "));
  tft.setCursor(OFFSET+OFFSETX, 60);
  tft.print(outstr);

  //Simulated temperature, change to your sensor
  dtostrf(temperature+30,4, 1, outstr);

  tft.setTextColor(Colors[3], Black);
  tft.setCursor(OFFSET, 90);
  tft.print(F("T04 "));
  tft.setCursor(OFFSET+OFFSETX, 90);
  tft.print(outstr);

  //Simulated temperature, change to your sensor
  dtostrf(temperature+40,4, 1, outstr);
  
  tft.setTextColor(Colors[4], Black);
  tft.setCursor(OFFSET, 120);
  tft.print(F("T05 "));
  tft.setCursor(OFFSET+OFFSETX, 120);
  tft.print(outstr);

  //Simulated temperature, change to your sensor
  dtostrf(temperature+50,4, 1, outstr);

  tft.setTextColor(Colors[5], Black);
  tft.setCursor(OFFSET, 150);
  tft.print(F("T06 "));
  tft.setCursor(OFFSET+OFFSETX, 150);
  tft.print(outstr);

  //Simulated temperature, change to your sensor
  dtostrf(temperature+60,4, 1, outstr);

  tft.setTextColor(Colors[6], Black);
  tft.setCursor(OFFSET, 180);
  tft.print(F("T07 "));
  tft.setCursor(OFFSET+OFFSETX, 180);
  tft.print(outstr);

  //Print the time and date
  tft.setTextColor(Colors[7], Black);
  tft.setCursor(OFFSET, 210);
  tft.print(hour(TimeNow));
  tft.print(F(":"));
  tft.print(minute(TimeNow));
  tft.print(F(":"));
  tft.print(second(TimeNow));
  tft.print(F(" "));
  tft.print(day(TimeNow));
  tft.print(F("/"));
  tft.print(month(TimeNow));
  tft.print(F("/"));
  tft.print(year(TimeNow)%100);
    }
  break;
  
  case 2 :
  {
    //Screen2, graph of the selected thermocouple
    
    tft.fillRect(0, 0, 320-1, 240-1, Black);
    tft.drawRect(0, 0, 320-1, 240-1, White);
    tft.setTextSize(2);
    tft.setTextColor(White, Black);
    tft.setTextSize(1);
    
    //Draw vertical lines to build the graph
    for (int i =0; i<=8; i++) {
     tft.drawFastVLine(40*i, 0, 239, White);
    }

    //Draw horizontal lines to build the graph
    for (int i = 0; i<= 6; i++) {
     tft.drawFastHLine(0,i*40, 319, White);
    }
    
    //Evaluate max = Stat[0], min = Stat[1], and mean value = Stat[2] of T[]
    //T[] = 100 * temperature
    Statistic(Selection);
    Serial.print(" S0 ");
    Serial.print(Stat[0]);
    Serial.print(" S1 ");
    Serial.print(Stat[1]);
    Serial.print(" S2 ");
    Serial.print(Stat[2]);
    Serial.print(" ");
    
    
    tft.setTextSize(2);
    tft.setCursor(204, 4);
    //Print the minimum value in the graph
    dtostrf(Stat[0]/100,2, 2, outstr);
    tft.print(outstr);
    tft.setCursor(204, 4+3*32-18);
    //Print the actual temperature
    dtostrf(temperature+10*Selection,2, 2, outstr);
    tft.print(outstr);
    tft.setCursor(204, 239-18-4);
    //Print the maximum value in the graph
    dtostrf(Stat[1]/100,2, 2, outstr);
    tft.print(outstr);
    tft.setCursor(10, 239-18-4);
    //Print the actual Selected Thermocouple
    tft.print(F("T0"));
    tft.print(Selection+1);
    
    //Draw points
    float shift = map(line,0,Size-1,0,319);


    //Draw the graph points, each point 9 pixels
    for (int i = line; i<=Size-1; i++) {
      //Data from line to the end of T[] are older, shift x axis left
      //Autoscale using map to graph dimensions map(value, fromLow, fromHigh, toLow, toHigh)
      tft.drawPixel(map(i,0,Size-1,0,319)-shift, map(T[i][Selection],Stat[0],Stat[1],0,239), Colors[Selection]);
      tft.drawPixel(map(i,0,Size-1,0,319)-shift, map(T[i][Selection],Stat[0],Stat[1],0,239)+1, Colors[Selection]);
      tft.drawPixel(map(i,0,Size-1,0,319)-shift, map(T[i][Selection],Stat[0],Stat[1],0,239)-1, Colors[Selection]);
      tft.drawPixel(map(i,0,Size-1,0,319)-shift+1, map(T[i][Selection],Stat[0],Stat[1],0,239), Colors[Selection]);
      tft.drawPixel(map(i,0,Size-1,0,319)-shift+1, map(T[i][Selection],Stat[0],Stat[1],0,239)+1, Colors[Selection]);
      tft.drawPixel(map(i,0,Size-1,0,319)-shift+1, map(T[i][Selection],Stat[0],Stat[1],0,239)-1, Colors[Selection]);
      tft.drawPixel(map(i,0,Size-1,0,319)-shift-1, map(T[i][Selection],Stat[0],Stat[1],0,239), Colors[Selection]);
      tft.drawPixel(map(i,0,Size-1,0,319)-shift-1, map(T[i][Selection],Stat[0],Stat[1],0,239)+1, Colors[Selection]);
      tft.drawPixel(map(i,0,Size-1,0,319)-shift-1, map(T[i][Selection],Stat[0],Stat[1],0,239)-1, Colors[Selection]);
      
    }
    for (int i = 0; i<=line-1; i++) {
      //Data from 0 to the line of T[] are newer, shift x axis right
      //Autoscale using map to graph dimensions
      tft.drawPixel(map(i,0,Size-1,0,319)+319-shift, map(T[i][Selection],Stat[0],Stat[1],0,239), Colors[Selection]);
      tft.drawPixel(map(i,0,Size-1,0,319)+319-shift, map(T[i][Selection],Stat[0],Stat[1],0,239)+1, Colors[Selection]);
      tft.drawPixel(map(i,0,Size-1,0,319)+319-shift, map(T[i][Selection],Stat[0],Stat[1],0,239)-1, Colors[Selection]);
      tft.drawPixel(map(i,0,Size-1,0,319)+319-shift+1, map(T[i][Selection],Stat[0],Stat[1],0,239), Colors[Selection]);
      tft.drawPixel(map(i,0,Size-1,0,319)+319-shift+1, map(T[i][Selection],Stat[0],Stat[1],0,239)+1, Colors[Selection]);
      tft.drawPixel(map(i,0,Size-1,0,319)+319-shift+1, map(T[i][Selection],Stat[0],Stat[1],0,239)-1, Colors[Selection]);
      tft.drawPixel(map(i,0,Size-1,0,319)+319-shift-1, map(T[i][Selection],Stat[0],Stat[1],0,239), Colors[Selection]);
      tft.drawPixel(map(i,0,Size-1,0,319)+319-shift-1, map(T[i][Selection],Stat[0],Stat[1],0,239)+1, Colors[Selection]);
      tft.drawPixel(map(i,0,Size-1,0,319)+319-shift-1, map(T[i][Selection],Stat[0],Stat[1],0,239)-1, Colors[Selection]);
    }
    //Delay before graph refresh
    delay(5000);
  }
  break;
  
  case 3 :
  {
    //Screen3
  tft.fillRect(0, 0, 320-1, 240-1, Black);
  tft.setTextSize(3);
  tft.setTextColor(Colors[0], Black);
  tft.setCursor(OFFSET, 3);
  tft.print(F("Cheap Data"));

  tft.setTextColor(Colors[1], Black);
  tft.setCursor(OFFSET, 30);
  tft.print(F("Logger V1.0"));

  tft.setTextColor(Colors[3], Black);
  tft.setCursor(OFFSET, 90);
  tft.print(F("See you soon "));

  tft.setTextColor(Colors[4], Black);
  tft.setCursor(OFFSET, 120);
  tft.print(F("with a new "));

  tft.setTextColor(Colors[5], Black);
  tft.setCursor(OFFSET, 150);
  tft.print(F("version, "));

  tft.setTextColor(Colors[6], Black);
  tft.setCursor(OFFSET+OFFSETX, 180);
  tft.print(F("Yiu"));

  tft.setTextColor(Colors[7], Black);
  tft.setCursor(OFFSET, 210);
  tft.print(F("30/11/2016"));
  delay(3000);
  
  }
  break;

  } //Close Switch
  
  if (TimeNow > PreviousTime+DeltaTime ) { 
    //in T[] save 100*temperature to keep the variable as int 
    T[line][0]=(int) (temperature*100+0.5);
    //Simulated temperature, change to your sensor
    T[line][1]=(int) ((temperature+10)*100+0.5);
    T[line][2]=(int) ((temperature+20)*100+0.5);
    T[line][3]=(int) ((temperature+30)*100+0.5);
    T[line][4]=(int) ((temperature+40)*100+0.5);
    T[line][5]=(int) ((temperature+50)*100+0.5);
    T[line][6]=(int) ((temperature+60)*100+0.5);
    if (line>=Size) {line=0;} else {line=line+1;}
    PreviousTime=TimeNow;
    Serial.print(" line ");
    Serial.print(line);
  }
  
  ScreenPrev = Screen;
  Serial.println(" done");
} //End main loop

void Statistic(int l) {
  int Max = 0;
  int Min = 32767;
  long Tot = 0;
  for (int i=0; i<= Size-1; i++) {
    if (T[i][l]>Max) {Max = T[i][l];}
    if (T[i][l]<Min) {Min = T[i][l];}
    Tot = Tot + T[i][l];
  }
  Stat[0]=Max;
  Stat[1]=Min;
  Stat[2]=Tot/Size;
}
