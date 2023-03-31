#include <Q2HX711.h>
#include "VirtualPanel.h"

const byte hx711_data_pin = A2;
const byte hx711_clock_pin = A3;

Q2HX711 hx711(hx711_data_pin, hx711_clock_pin);

float Weight = 0;
float Tare = 0;

float MeanAlpha = 0.25;
float VarianceAlpha = 0.5;

float AMean = 0.0;
float ADelta = 0.0;
float AVariance = .0;

float RMean = 0.0;
float BMean = 0.0;
int CycleCount = 0;

float PMax = 0.0;
float PMin = 1000000.0;

void setup() 
{
  Panel.begin(); // init port and protocol
  Weight = hx711.read() * 0.000547;
  Tare = Weight;
}

void loop() 
{
  Panel.receive(); // handle panel events form the panel (must be in the loop)
  Weight = (float) hx711.read() * 0.000547;

//  if(abs(Weight - BMean) < 0.1)
//    RMean = ((RMean * 29.0) + Weight) / 30.0;
//  else
//    RMean = Weight;
  
  if(abs(Weight-BMean) < 0.1) 
  {
    if(PMax < Weight) PMax = Weight;
    if(PMin > Weight) PMin = Weight;
    BMean = (PMax + PMin) / 2;
   } 
   else 
   {
     BMean = Weight;
     PMax = BMean;
     PMin = BMean;
   }



//  Weight = AlphaMean((float) hx711.read());
//  for(int i = 0; i < 10; i++)
//    Weight += hx711.read();
//  Weight /= 10;
}

void PanelCallback(vp_channel event) 
{ 
  switch (event) 
  {
    case PanelConnected: // receive panel connected event
      Panel.send(ApplicationName,"Pajama Scale"); 
      Panel.send(Button_8, F("tare"));
      Panel.send(DynamicDisplay, 250);
      break;

    case Button_8:
      Tare = Weight;
      break;

    case DynamicDisplay:
      float DWeight = BMean - Tare; // * 0.000547) - 0.55625
      if( abs(DWeight) < 0.05) DWeight = 0.0;
    
      Panel.sendf(Display_1, "%s g", _FString(DWeight, 4, 2));
      Panel.sendf(MonitorField_1, "Max: %s g", _FString(PMax - Tare, 5, 3));
      Panel.sendf(MonitorField_2, "Min: %s g", _FString(PMin - Tare, 5, 3));

      break;

    default: break;

  }
}



float AlphaMean(float NewValue)
{
  float ADelta = (float)NewValue - AMean;
  
//  if (ADelta < sqrt(AVariance)*3) 
    AMean = AlphaFilter(AMean, (float)NewValue, MeanAlpha); 
//  else
//    AMean = AlphaFilter(AMean, (float)NewValue, MeanAlpha/10.0); 

  AVariance = AlphaFilter(AVariance, sq(ADelta), VarianceAlpha);

  return AMean;
}


float AlphaFilter(float Mean, float Value, float Alpha) 
{
   return (Mean + ((Value - Mean) * Alpha));
}
