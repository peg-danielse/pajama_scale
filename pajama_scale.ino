#include <Q2HX711.h>

const byte data_pin = A2;
const byte clock_pin = A3;

Q2HX711 hx711(data_pin, clock_pin);

void setup()
{
       Serial.begin(115200);
}

void loop()
{
        Serial.println(hx711.read());
        delay(50);
}
