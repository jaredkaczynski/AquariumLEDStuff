


#include <VirtualWire.h>


const int receive_pin = 2;
const int LEDPins[6] = {3,5,6,10,11,12};
int LEDValue[6]={200,255,100,1,1,1}; //5000k blue 3000k
void setup()
{
    Serial.begin(9600);	// Debugging only
    Serial.println("setup");
    for(int i = 0; i < 6; i++)
    {
      pinMode(LEDPins[i], OUTPUT);
      Serial.println("setup LED Output" + i);
    }
    // Initialise the IO and ISR
    vw_setup(2000);	 // Bits per sec
    vw_set_rx_pin(receive_pin);
    vw_rx_start();       // Start the receiver PLL running
}

void loop()
{
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
      analogWrite(LEDValue[0],1);
      int nb = atoi(LEDChannel);
      int n = atoi(buffer);
      if(nb > 0 && nb < 7){
      if(n > -1 && n < 256){
      LEDValue[nb-1]=n;
      Serial.println("PWM Pin" + nb);
      Serial.println("PWM Value" + atoi(buffer));
       }
     }
    }
    for(int i = 0; i<6; i++){
       analogWrite(LEDPins[i],LEDValue[i]);
    } 
}
