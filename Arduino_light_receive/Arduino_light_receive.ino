
#include <VirtualWire.h>


const int receive_pin = 2;
const int LEDPins[6] = {3,5,6,10,11,12};
//int LEDValue[6]={100,150,50,1,1,1}; //5000k blue 3000k renamed to currentvalue
int goalValue[6];
int currentValue[6]={100,150,50,1,1,1}; //5000k blue 3000k
void setup()
{
    Serial.begin(9600);  // Debugging only
    Serial.println("setup");
    for(int i = 0; i < 6; i++)
    {
      pinMode(LEDPins[i], OUTPUT);
      Serial.println("setup LED Output" + i);
    }
    // Initialise the IO and ISR
    vw_setup(2000);  // Bits per sec
    vw_set_rx_pin(receive_pin);
    vw_rx_start();       // Start the receiver PLL running
}


void check_messages(){
    uint8_t buf[VW_MAX_MESSAGE_LEN];
    uint8_t buflen = VW_MAX_MESSAGE_LEN;
    Serial.println("Still running!");
    if (vw_get_message(buf, &buflen)) // Non-blocking
    {
      char buffer[4];
      buffer[0] = buf[1];
      buffer[1] = buf[2];
      buffer[2] = buf[3];
      buffer[3] = '\0';
      char LEDChannel[2];
      LEDChannel[0] = buf[0];
      LEDChannel[1] = '\0';
      Serial.println("PWM Value Pre value" + atoi(buffer));
      //analogWrite(currentValue[0],1); should this exist?
      int nb = atoi(LEDChannel);
      int n = atoi(buffer);
      if(nb > 0 && nb < 7){
      if(n > -1 && n < 256){
      currentValue[nb-1]=n;
      Serial.println("PWM Pin" + nb);
      Serial.println("PWM Value" + atoi(buffer));
       }
     }
    }
}

//Handle light logic to actually change the light value
//This results in smoother transitions between lighting levels
//This will always be the way levels change utilizing goalValues as the way for changes to occur elsewhere

void do_light(){
  for(int i = 0; i<6; i++){
       if(goalValue[i]!=currentValue[i]){
           int delta = 1; // value to change the currentvalue by
           if(goalValue[i]<currentValue[i]){
               delta = -1;
           }
           currentValue[i]+=delta;
           analogWrite(LEDPins[i],currentValue[i]);
       }
    } 
}

void loop()
{
    check_messages();
    do_light();
    
}
