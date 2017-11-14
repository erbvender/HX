#include <HX.h>

HX loadcell(5, 6, 7);  //out, clk, pow pins

void setup()
{
  Serial.begin(9600); Serial.println("test serial");
}

void loop()
{
  Serial.print(loadcell.get_counts(5, 2, 2, 10, 2, 0));Serial.print("\t");
  Serial.print(loadcell.RMS_TRIES); Serial.print("\t");
  Serial.println(loadcell.rms);
  delay(4000);
}
