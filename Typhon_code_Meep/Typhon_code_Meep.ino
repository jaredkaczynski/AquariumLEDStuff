/*
This sketch was made for use with the Typhon LED Controller by A. Rajamani.
Very special thanks to jedimasterben of nano-reef.com and stevesleds.com for supplying the Typhon controller for testing.
 
 I also ask that you credit me for making this code if you post it on other boards or share it with any other person or persons.
 
 A. Rajamani is not responsible for any personal or other injuries or damage caused by the use of this code or the way it is applied.
 */
#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include "math.h"
#include <TimeLib.h>


//lcd stuff

/*
show works like this:
  menu is 0-98
  lighting screen 1 is 99-198, with 100 being dawn on hour, 101 being minute, etc.
  lighting screen 2 is 199-298
  lighting screen 3 is 299-398
  lighting screen 4 is 399-498
  acclimation screen is 500-599
  time screen is 600-699
  light test screen is 700-799 with 701 channel 1, 702 channel 2, etc.
  lightning storms are 800-899
 
 Lighting is set up that way so hour 0 corresponds to a multiple of 100, to utilize a modulo function easier.
 
EEPROM is used this way:
Lighting
  1-6 is channel 1 dimming values
  7-12 is chanel 2 dimming values
  13-18 is channel 3 dimming values
  19-24 is channel 4 dimming values
*/
int show = 0; //Variable for display showing
int absvalue;

//Things to detect rollovers of the millis() function
boolean rolled;

int menucount;
int selectcount;
int pluscount;
int minuscount;

//Time variables
int t[7];

byte lastsec;
int dsecond;        // 0-59
int dminute;        // 0-59
int dhour;          // 1-23
int DOW;            // 1-7
int ddate;          // 1-31
int dmonth;         // 1-12
int dyear;

struct lightingchannel
{
  byte dim;
  byte goalvalue;
  byte test;
  byte value[7]; //0 is dawnstart, 1 is dawnend, 2 is duskstart, 3 is duskend, 4 is moonlight. 5 is parabola or not.
  byte time[9]; //0 is dawn start hour, 1 is minute, 2 is dawn end hour, 3 is minute, 4 dusk start hour, 5 is minute, 6 is dusk end hour, 7 is minute.
  long dawntime;
  long nighttime;
  long dawncurrent;
  long nightcurrent;
  long current;
  float dawnmeep;
  float nightmeep;
  int pin; 
};

//Number of lighting channels
const int channel_count = 6;

lightingchannel ch[channel_count];

//pins for lights
const byte onepin = 3;
const byte twopin = 5;
const byte threepin = 6;
const byte fourpin = 10;

//acclimation stuff
char accpercent;
char accdays;
char daycount;



void setup()
{
  lightSetup();
  timeSetup();
  setTime(22,5,0,15,1,17); // Set Time
  Serial.begin(9600);
  Serial.println("Setup 1");                               
}

void timeSetup()
{
  dsecond = second();         // 0-59
  dminute = minute();         // 0-59
  dhour = hour();          // 1-23
  DOW = day()%7;             // 1-7
  ddate = day();          // 1-31
  dmonth = month();          // 1-12
  dyear = year();
  lastsec = dsecond;
}

void lightSetup()
{
  ch[0].pin = 3;
  ch[1].pin = 5;
  ch[2].pin = 6;
  ch[3].pin = 10;
  //Starting acclimation point
  accdays = 0;
  daycount = 0;
  //Reading the EEPROM memory in order to assign ighting values to the lighting arrays at startup.
  assignValues();
  lightingcalc(0);
  lightingcalc(1);
  lightingcalc(2);
}

void assignValues(){
  //0 is dawnstart, 1 is dawnend, 2 is duskstart, 3 is duskend, 4 is moonlight. 5 is parabola or not.
  //0 is dawn start hour, 1 is minute, 2 is dawn end hour, 3 is minute, 4 dusk start hour, 5 is minute, 6 is dusk end h
  //ch[0].value = {10,155,155,10,10,1,0};
  ch[0].value[0] = 10;
  ch[0].value[1] = 155;
  ch[0].value[2] = 155;
  ch[0].value[3] = 10;
  ch[0].value[4] = 0;
  ch[0].value[5] = 1;
  //ch[1].value = {10,120,120,10,10,1,0};
  ch[1].value[0] = 10;
  ch[1].value[1] = 120;
  ch[1].value[2] = 120;
  ch[1].value[3] = 10;
  ch[1].value[4] = 3;
  ch[1].value[5] = 1;
  //ch[2].value = {10,65,65,10,10,1,0};
  ch[2].value[0] = 10;
  ch[2].value[1] = 65;
  ch[2].value[2] = 65;
  ch[2].value[3] = 10;
  ch[2].value[4] = 0;
  ch[2].value[5] = 1;
  //ch[0].time = {5,30,9,30,18,0,19,30,0};
  ch[0].time[0] = 5;
  ch[0].time[1] = 30;
  ch[0].time[2] = 9;
  ch[0].time[3] = 30;
  ch[0].time[4] = 18;
  ch[0].time[5] = 0;
  ch[0].time[6] = 19;
  ch[0].time[7] = 30;
  
  //ch[1].time = {6,30,10,30,17,0,20,0,0};
  ch[1].time[0] = 6;
  ch[1].time[1] = 30;
  ch[1].time[2] = 10;
  ch[1].time[3] = 30;
  ch[1].time[4] = 17;
  ch[1].time[5] = 0;
  ch[1].time[6] = 20;
  ch[1].time[7] = 0;
  
  //ch[2].time = {5,30,8,30,18,0,19,0,0};
  ch[2].time[0] = 5;
  ch[2].time[1] = 30;
  ch[2].time[2] = 8;
  ch[2].time[3] = 30;
  ch[2].time[4] = 18;
  ch[2].time[5] = 0;
  ch[2].time[6] = 19;
  ch[2].time[7] = 0;
}

void loop()
{
  //timeloop does most of the major light work
  timeLoop();
  //change brightness smoothly transitions between light changes 
  //This is for future use to manually control light over wifi or similar
  change_brightness();
}

void timeLoop()
{
  dsecond = second();         // 0-59
  dminute = minute();         // 0-59
  dhour = hour();          // 1-23
  DOW = day()%7;             // 1-7
  ddate = day();          // 1-31
  dmonth = month();          // 1-12
  dyear = year();
  
  //Run once a second
  if(lastsec != dsecond)
  {
    lastsec = dsecond;
    lightDisplay();
  }
}

void lightDisplay()
{
  for(int i = 0; i <= 3; i++)
  {
    lightLoop(i);
  }
}

//Handle light logic to actually change the light value
//This results in smoother transitions between lighting levels
//This will always be the way levels change utilizing goalValues as the way for changes to occur elsewhere

void change_brightness(){
  for(int i = 0; i<channel_count; i++){
       if(ch[i].dim!=ch[i].goalvalue){
           int delta = 1; // value to change the currentvalue by
           if(ch[i].goalvalue<ch[i].dim){ //if the goal is less than the current value subtract obviously
               delta = -1;
           }
           //Speed up if there's a large delta
           if(ch[i].goalvalue-ch[i].dim > 20){
               delta*=5;
           }
           //adjust the current light level 
           ch[i].dim+=delta;
           analogWrite(ch[i].pin,ch[i].dim);
       }
    } 
}

void lightLoop(int b)
{
  //calculating acclimation
  if(accdays > daycount && accdays != 0)
  {
    accpercent = map(daycount, 0, accdays, 0, 100);
  }
  else
  {
    accpercent = 100;
  }
    
  //Lighting calculation main
  long cminute = long(dhour)*3600L + long(dminute)*60L + long(dsecond); //time in seconds
  if(cminute < long(ch[b].time[0])*3600L + long(ch[b].time[1])*60L || cminute >= long(ch[b].time[6])*3600L + long(ch[b].time[7])*60L) //nighttime
  {
    ch[b].dim = ch[b].value[4];
  }
  
  if(ch[b].value[5])
  {
    if((cminute >= long(ch[b].time[0])*3600L + long(ch[b].time[1])*60L) && cminute < (long(ch[b].time[2])*3600L + long(ch[b].time[3])*60L)) //dawn
    {
      ch[b].dawncurrent = cminute - (long(ch[b].time[0])*3600L + long(ch[b].time[1])*60L);
      ch[b].dim = ch[b].value[1] - (float(ch[b].value[1]-ch[b].value[0])/ pow(ch[b].dawnmeep, pow(float(ch[b].dawncurrent), 2.0)));
      if(ch[b].dim > ch[b].value[1])
      {
        ch[b].dim = ch[b].value[1];
      }
    }
    if(cminute >= (long(ch[b].time[2])*3600L + long(ch[b].time[3])*60L) && cminute < (long(ch[b].time[4])*3600L + long(ch[b].time[5])*60L)) //noon
    {
      ch[b].dim = map(cminute, (ch[b].time[2]*3600 + ch[b].time[3]*60), (ch[b].time[4]*3600 + ch[b].time[5]*60), ch[b].value[1], ch[b].value[2]);
    }
    if(cminute >= (long(ch[b].time[4])*3600L + long(ch[b].time[5])*60L) && cminute < (long(ch[b].time[6])*3600L + long(ch[b].time[7])*60L)) //dusk
    {
      ch[b].nightcurrent = cminute - (long(ch[b].time[4])*3600L + long(ch[b].time[5])*60L);
      ch[b].dim = ch[b].value[3] - (float(ch[b].value[3]-ch[b].value[2])/ pow(ch[b].nightmeep, pow(float(ch[b].nightcurrent), 2.0)));
      if(ch[b].dim < ch[b].value[3])
      {
        ch[b].dim = ch[b].value[3];
      }
    }
  }
  else //if not a bell curve
  {
    if(cminute >= (long(ch[b].time[0])*3600L + long(ch[b].time[1])*60L) && cminute < (long(ch[b].time[2])*3600L + long(ch[b].time[3])*60L)) //dawn
    {
      ch[b].dim = map(cminute, (long(ch[b].time[0])*3600L + long(ch[b].time[1])*60L), (long(ch[b].time[2])*3600L + long(ch[b].time[3])*60L), ch[b].value[0], ch[b].value[1]);
    }
    if(cminute >= (long(ch[b].time[2])*3600L + long(ch[b].time[3])*60L) && cminute < (long(ch[b].time[4])*3600L + long(ch[b].time[5])*60L)) //noon
    {
      ch[b].dim = map(cminute, (long(ch[b].time[2])*3600L + long(ch[b].time[3])*60L), (long(ch[b].time[4])*3600L + long(ch[b].time[5])*60L), ch[b].value[1], ch[b].value[2]);
    }
    if(cminute >= (long(ch[b].time[4])*3600L + long(ch[b].time[5])*60L) && cminute < (long(ch[b].time[6])*3600L + long(ch[b].time[7])*60L)) //dusk
    {
      ch[b].dim = map(cminute, (long(ch[b].time[4])*3600L + long(ch[b].time[5])*60L), (long(ch[b].time[6])*3600L + long(ch[b].time[7])*60L), ch[b].value[2], ch[b].value[3]);
    }
  }  
  if(show >= 700 && show <= 704) //lighting test screen
  {
    ch[b].dim = ch[b].test;
  }
  //setting the channel goal value
  ch[b].goalvalue = (ch[b].dim*accpercent/100);
  //moved the lighting adjustment to a smooth management method to allow smooth manual adjustment
  //analogWrite(ch[b].pin, (ch[b].dim*accpercent/100));
  Serial.println(ch[b].pin);
  Serial.println(ch[b].dim*accpercent/100);
  Serial.println(second());
}
  
void lightingcalc(int c)
{
  ch[c].dawntime = (ch[c].time[2]*3600 + ch[c].time[3]*60)-(ch[c].time[0]*3600 + ch[c].time[1]*60);
  ch[c].nighttime = (ch[c].time[6]*3600 + ch[c].time[7]*60)-(ch[c].time[4]*3600 + ch[c].time[5]*60);
  ch[c].dawnmeep = pow(float(ch[c].value[1]), 1.0/pow(float(ch[c].dawntime), 2.0)); //b = pow(float(maxdim), 1.0/pow(float(dawncycletime), 2.0));
  ch[c].nightmeep = pow(float(ch[c].value[2]), 1.0/pow(float(ch[c].nighttime), 2.0));
}
