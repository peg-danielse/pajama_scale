#include <Q2HX711.h>
#include "VirtualPanel.h"

const byte hx711_data_pin = A2;
const byte hx711_clock_pin = A3;

Q2HX711 hx711(hx711_data_pin, hx711_clock_pin);

float Weight = 0;
float Tare = 0;

void setup() 
{
  Panel.begin(); // init port and protocol
  Weight = hx711.read();
  Tare = Weight;
}

void loop() 
{
  Panel.receive(); // handle panel events form the panel (must be in the loop)
  
  for(int i = 0; i < 10; i++)
    Weight += hx711.read();
  Weight /= 10;
}

void PanelCallback(vp_channel event) 
{ 
  switch (event) 
  {
    case PanelConnected: // receive panel connected event
      Panel.send(ApplicationName,"Pajama Scale"); 
      Panel.send(Button_8, F("tare"));
      Panel.send(DynamicDisplay, true);
      break;

    case Button_8:
      Tare = Weight;
      break;

    case DynamicDisplay:
      float DWeight = ((Weight - Tare) * 0.000547); //- 0.55625
    
      Panel.sendf(Display_1,"%s g", _FString(DWeight, 4, 2));
      break;

    default: break;

  }
}
