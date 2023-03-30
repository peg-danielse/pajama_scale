#include <Q2HX711.h>
#include "VirtualPanel.h"

const byte hx711_data_pin = A2;
const byte hx711_clock_pin = A3;

Q2HX711 hx711(hx711_data_pin, hx711_clock_pin);

float Weight = 0.0;
float Tare = 0.0;

void setup() 
{
  Panel.begin(); // init port and protocol
}

void loop() 
{
  Panel.receive(); // handle panel events form the panel (must be in the loop)

  Weight = ((float)hx711.read() / 1000.0);
  Panel.sendf(Display_1,"%s", _FString(Weight - Tare,1,0));
  delay(500);
}

void PanelCallback(vp_channel event) 
{ 
  switch (event) 
  {
    case PanelConnected: // receive panel connected event
      Panel.send(ApplicationName,"Pyama Scale"); 
      Panel.send(Button_8, F("tare"));
      break;

    case Button_8:
      Tare = Weight;

    default: break;

  }
}
