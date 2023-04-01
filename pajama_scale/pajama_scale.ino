#include <Q2HX711.h>
#include "VirtualPanel.h"

const byte hx711_data_pin  = A2; 
const byte hx711_clock_pin = A3;

Q2HX711 hx711(hx711_data_pin, hx711_clock_pin);

float Weight = 0.0; // Weight variable
float Tare   = 0.0; // Tare variable

//--------------------------------------------------------------------------------
void setup() 
{
  Panel.begin(); // init port and protocol
  Tare = GetBalancedWeight(); // init Tare
}

//--------------------------------------------------------------------------------
void loop() 
{
  Panel.receive(); // handle panel events from the panel (must be in the loop)
  Weight = GetBalancedWeight(); // get balanced weight
  if(Weight - Tare < 0.06 && Weight - Tare > -0.10) Tare = Weight; // auto tare if close to 0.0
}

//--------------------------------------------------------------------------------
float GetBalancedWeight() 
{ 
  const  float LoadCellGradient = 0.000547; // HX711 loadcell value to grams gradient
         float MWeight = 0.0; // Mesured weigt
  static float BWeight  = 0.0; // Balanced Weight
  static float PMax = 0.0; // Max BWeight Noise
  static float PMin = 1000000.0; // Min BWeight Noise
  static float PCorr = 0.000243; // Min/Max slow correct
  
  MWeight = (float) hx711.read() * LoadCellGradient;
  
  if(abs(MWeight - BWeight) < 0.1) // if measured value close to (previous) Balanced Weight
  {
    PMax = PMax - PCorr; // minute decrease of PMax works away unballanced outliers
    PMin = PMin + PCorr; // minute increase of PMin works away unballanced outliers
    if(PMax < MWeight) PMax = MWeight; // if measured weight is larger then PMax set PMax
    if(PMin > MWeight) PMin = MWeight; // if measured weight is smaller then PMin set PMin
    BWeight = (PMax + PMin) / 2; // Average between PMax and PMin is our Balanced Weight
   } 
   else 
   { // measured value is not close to balanced weight
     BWeight = MWeight; // set balanced weight to measured weight 
     PMax = BWeight; // set PMax to measured weight  
     PMin = BWeight; // set PMin to measured weight 
   }
   
   Panel.sendf(MonitorField_3, F("Max     : %s g"), _FString(PMax - BWeight, 6, 4));
   Panel.sendf(MonitorField_4, F("Min     : %s g"), _FString(PMin - BWeight, 6, 4));
  
   return BWeight; // Balanced weight output
}


//--------------------------------------------------------------------------------
void PanelCallback(vp_channel event) 
{ 
  switch (event) 
  {
    case PanelConnected: // receive panel connected event: Init panel
      Panel.send(ApplicationName, F("Pajama Scale")); // Set Application name
      Panel.send(Display_1, F("$BIG")); // Set Application name
      Panel.send(Button_8, F("tare")); // Set tare weight button
      Panel.send(DynamicDisplay, 200); // Display update each 200ms
      break;

    case Button_8:
      Tare = Weight;
      break;

    case DynamicDisplay:
      float DWeight = Weight - Tare; // Calculate brute weight
      if( abs(DWeight) < 0.01) DWeight = 0.0; // clip noise

      Panel.sendf(Display_1, F("%s g"), _FString(DWeight, 3, 1));
      Panel.sendf(MonitorField_1, F("Tare     : %s g"), _FString(Tare, 6, 4));
      Panel.sendf(MonitorField_2, F("Weight  : %s g"), _FString(Weight, 6, 4));
      break;

    default: break;
  }
}
