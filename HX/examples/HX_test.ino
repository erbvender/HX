#include <HX.h>


HX loadcell(5, 6, 7);


void setup()
{
  Serial.begin(9600); Serial.println("test serial");


}

void loop()
{
//  Serial.println(a);
//  Serial.println(loadcell.rms);
 // loadcell.wake_hx();
 Serial.print(loadcell.get_counts(5, 2, 2, 10, 2));Serial.print("\t");
 Serial.print(loadcell.RMS_TRIES); Serial.print("\t");
 Serial.println(loadcell.rms);
//  loadcell.sleep_hx();
  delay(4000);
}
