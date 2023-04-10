#include <Q2HX711.h>
#include "VirtualPanel.h"

const byte ArduinoLed = 13;
const byte hx711_data_pin  = A2; 
const byte hx711_clock_pin = A3;

const  float LoadCellGradient = 0.000533183; // HX711 loadcell value to grams gradient 0.000533183 0.0005335

const int WeightBufferSize = 30;
float     WeightBuffer[WeightBufferSize];
int       WeightBufferIdx = 0;
bool      WeightBufferMax = false;
float     oldMean = 0;
float     oldSD = 0;

bool      Save = false;

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
  //if(Weight - Tare < 0.05 && Weight - Tare > -0.10) Tare = Weight; // auto tare if close to 0.0 (temporary temp. compenstation)
}

//--------------------------------------------------------------------------------
float GetBalancedWeight() 
{ 
         float MWeight = 0.0; // Mesured weigt
  static float BWeight  = 0.0; // Balanced Weight
  static float PMax = 0.0; // Max BWeight Noise
  static float PMin = 1000000.0; // Min BWeight Noise
  static float PCorr = 0.000244; // Min/Max slow correct

  static float AWeight = 0.0;
  static float ACount = 0;
      uint32_t MeasureTime = 0;

  MeasureTime = millis();
  
  MWeight = (float) hx711.read() * LoadCellGradient;
  // MWeight = (float) hx711.read();
  
  if(abs(MWeight - BWeight) < 0.1) // if measured value close to (previous) Balanced Weight
  {
    PMax = PMax - PCorr; // minute decrease of PMax works away unballanced outliers
    PMin = PMin + PCorr; // minute increase of PMin works away unballanced outliers
    
    if(PMax < MWeight) PMax = MWeight; // if measured weight is larger then PMax set PMax
    if(PMin > MWeight) PMin = MWeight; // if measured weight is smaller then PMin set PMin
   
    BWeight = (PMax + PMin) / 2; // Average between PMax and PMin is our Balanced Weight

    AWeight = ((AWeight * ACount) + MWeight) / (ACount + 1);
    ACount++;
    
    if(ACount > 60) 
    {
      BWeight = AWeight;
      Panel.send(Led_12, "$GREEN");
    }
    else if (ACount > 30)
    {
      BWeight = AWeight;
      Panel.send(Led_12, "$YELLOW");
    }
    else {
      Panel.send(Led_12, "$ORANGE");
    }
    
//    WeightBuffer[WeightBufferIdx++] = MWeight;
//    
//    if( WeightBufferIdx > WeightBufferSize)
//    { 
//      WeightBufferIdx = 0;
//      WeightBufferMax = true;
//      float newMean = Mean(WeightBuffer, WeightBufferSize);
//      float newSD = SD(WeightBuffer, WeightBufferSize);
//     
//      float statistic = TTest(newMean, oldMean, newSD, oldSD, WeightBufferSize);
//      
//     Panel.sendf(MonitorField_3, F("statistic     : %s"), _FString(statistic , 8, 6));
//     Panel.sendf(MonitorField_4, F("decision     : %d"), interpert(statistic, 2.0));
//     Panel.sendf(MonitorField_5, F("oldMean     : %s"), _FString(oldMean - Tare , 6, 4));
//     Panel.sendf(MonitorField_6, F("newMean     : %s"), _FString(newMean - Tare , 6, 4));
//
//     if( interpert(statistic, 3.1)) {
//        oldMean = newMean;
//        oldSD = newSD;
//     }
//    }

//    if(WeightBufferMax)
//    {
//      BWeight = oldMean;
//    }
//    else 
//    {
//      BWeight = (PMax + PMin) / 2;
//    }
    
   } 
   else 
   { // measured value is not close to balanced weight
     BWeight = MWeight; // set balanced weight to measured weight 
     PMax = BWeight; // set PMax to measured weight  
     PMin = BWeight; // set PMin to measured weight 
     ACount = 0.0;
     AWeight = 0.0;
     WeightBufferIdx = 0;
     WeightBufferMax = false;
     Panel.send(Led_12, "$OFF");
     
//   Panel.sendf(MonitorField_3, F("Max     : %s g"), _FString(PMax - BWeight, 6, 4));
//   Panel.sendf(MonitorField_4, F("Min     : %s g"), _FString(PMin - BWeight, 6, 4));
//   Panel.sendf(MonitorField_6, F("Measure Time: %lu ms"), millis() - MeasureTime);
   }
   
   return BWeight; // Balanced weight output
}

float Sum(float *buffer, int length)
{
 float sum = 0;
 
 for(int i = 0; i < length; i++) 
   sum += buffer[i];

 return sum;
}


float sqSum(float *buffer, int length)
{
 float sum = 0;
 
 for(int i = 0; i < length; i++) 
   sum += sq(buffer[i]);

 return sum;
}

float Mean(float *buffer, int length)
{
  return Sum(buffer, length)/length;
}

float SD(float *buffer, int length)
{
  float mean = Mean(buffer, length);
  float sumsq = 0;
  
  for(int i = 0; i < length; i++) 
    sumsq += sq(buffer[i] - mean);

  return sqrt(sumsq/length);
}

float TTest(float aMean, float bMean, float aSD, float bSD, int count) 
{

  float Sp = sqrt((((float) count - 1) * sq(aSD) + ((float) count - 1) * sq(bSD)) / (float) (count + count - 2));
  float statistic = (aMean - bMean) / (Sp * sqrt((1/(float)count) + (1/(float)count)));
  
  return statistic;
}

bool interpert(float statistic, float critical)
{
  return abs(statistic) > critical;
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
      Panel.send(Button_11, F("save\n30"));
      Panel.send(Monitor, true);
      Panel.send(DynamicDisplay, 200); // Display update each 200ms
      break;

    case Button_8:
      Tare = Weight;
      break;

    case Button_11:
      Panel.send(OpenFile_1, ".txt"); // Open file using dialog
      Save = true;
      break;

    case OpenFile_1:
       for(int i=0; i<30; i++)
       {
         // Panel.sendf(WriteLineFile_1, "%s", _FString((float) hx711.read() * 0.000547, 6,4));
         Panel.sendf(WriteLineFile_1, "%s", _FString((float) hx711.read(), 6,4));
       }
       break;
       
    case DynamicDisplay:
      float DWeight = Weight - Tare; // Calculate brute weight
      if( abs(DWeight) < 0.01) DWeight = 0.0; // clip noise

      Panel.sendf(Display_1, F("%s g"), _FString(DWeight, 4, 2));
      Panel.sendf(MonitorField_1, F("Tare     : %s g"), _FString(Tare, 6, 4));
      Panel.sendf(MonitorField_2, F("Weight  : %s g"), _FString(Weight, 6, 4));
      break;

    default: break;
  }
}
