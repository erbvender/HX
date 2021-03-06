
//avg_nr  avgSize snr? try

#include "Arduino.h"
#include "HX.h"


#if ARDUINO_VERSION <= 106
    void yield(void){};
#endif

HX::HX(byte out_pin, byte sck_pin, byte pow_pin)
{
// long result[10];
  begin(out_pin, sck_pin, pow_pin);
}

HX::HX()  {}
HX::~HX() {}

void HX::begin(byte out_pin, byte sck_pin, byte pow_pin)
{
  POW_PIN = pow_pin;
  SCK_PIN = sck_pin;
  OUT_PIN = out_pin;

  pinMode(POW_PIN, OUTPUT);
  pinMode(SCK_PIN, OUTPUT);
  pinMode(OUT_PIN, INPUT);
}

//Limits noice to "RMS_LIM" in bits
//If noice higher than "RMS_LIM" it tries "RMS_TRY" nr of times
//Result is shifted "SNR" nr of bits to get stable counts
//Power consumption is traded with resolution
//Selfheating may cause small drift
int HX::get_counts(byte RMS_LIM, byte SNR, byte RMS_TRY, byte AVG_NR, float cal)
{
  wake_hx(); 
  
  int nr = 0; 
  RMS_TRIES = 0; 
  rms = (1 << (RMS_LIM + 2));

  while((rms > (1 << RMS_LIM)) && (nr < RMS_TRY))
  {

    ESP.wdtFeed();
    avg = (long)(Read_hx(AVG_NR)); 
    rms = (int)RMS();
    nr++; 
    RMS_TRIES++;
  }

//  nr = 0;
//  while((rms > (1 << (RMS_LIM + 1))) && (nr < RMS_TRY))
//  {
//    avg = (long)(Read_hx(AVG_NR)); 
//    rms = (int)RMS();
//    nr++; 
//    RMS_TRIES++;
//  }
//
//  nr = 0;
//  while((rms > (1 << (RMS_LIM + 2))) && (nr < RMS_TRY))
//  {
//    avg = (long)(Read_hx(AVG_NR)); 
//    rms = (int)RMS();
//    nr++; 
//    RMS_TRIES++;
//  } 
  
  sleep_hx();
  avg = avg >> RMS_LIM;//funkar inte om rms_lim är för lågt kanske
  int avg2 = ((int)avg >> SNR);
  if(cal > 0){(avg2 = (int)(avg2 * cal);}
  return avg2;
}

long HX::RMS()
{  

  long bufferSumDiffSq = 0;
  for(int i = 0; i < 10; i++)
  {
    bufferSumDiffSq += sq(avg - result[i]);
  }
  return sqrt(bufferSumDiffSq/10);
}

bool HX::is_ready() {return (digitalRead(OUT_PIN) == 0);}

//To make it lowpower the powersupply has to be turned off (200uA)
//and clockPin has to be switched to input_pullup (>500uA)
//1 uA powerdown only acounts for the chip, not pcb's with the chip
void HX::sleep_hx()
{
  digitalWrite(SCK_PIN, LOW); digitalWrite(SCK_PIN,HIGH);
  digitalWrite(POW_PIN , LOW);  
  pinMode(SCK_PIN, INPUT_PULLUP);  
}  

void HX::wake_hx() 
{
  pinMode(SCK_PIN, OUTPUT); digitalWrite(SCK_PIN , HIGH);  
  digitalWrite(POW_PIN , HIGH);// delay(200);
  digitalWrite(SCK_PIN, LOW);
  for(int i = 0; i < 6; i++){getValue();} 
}

//Make AVG_SIZE nr of averages
//And optional spikefilter if AVG_NR is bigger than 3 (smallest and higest nr is discharged)
long HX::Read_hx(byte AVG_NR)
{

  long bufferSum = 0;
  for(int i = 0; i < 10; i++)
  {
    if(AVG_NR < 3){result[i] = getValue();}
    else          {result[i] = getValue_avg(AVG_NR);}
    bufferSum += result[i];
  }  

//  minmax(AVG);  enob?
  return (long)(bufferSum / 10);
} 

long HX::getValue()
{ 
  while (!is_ready()){yield();}
  byte data[3];

//  for (byte j = 3; j--;) {
//      data[j] = shiftIn(OUT_PIN,CLOCK_PIN, MSBFIRST);  

//unsigned long value = 0;
//uint8_t data[3] = { 0 };
//uint8_t filler = 0x00;
//  data[2] = shiftIn(DOUT, PD_SCK, MSBFIRST);
//  data[1] = shiftIn(DOUT, PD_SCK, MSBFIRST);
//  data[0] = shiftIn(DOUT, PD_SCK, MSBFIRST);
  for (byte j = 3; j--;)
  {
    for (byte i = 8; i--;)
    {
      digitalWrite(SCK_PIN, HIGH);
      bitWrite(data[j], i, digitalRead(OUT_PIN));
      digitalWrite(SCK_PIN, LOW);
    }
  }
  
  digitalWrite(SCK_PIN, HIGH); digitalWrite(SCK_PIN, LOW);

//  if   (data[2] & 0x80) {filler = 0xFF;}
//  else                  {filler = 0x00;}
  data[2] ^= 0x80;


//    data[2] ^= 0x80;
//  return ((uint32_t) data[2] << 16) | ((uint32_t) data[1] << 8) | (uint32_t) data[0];

  return (((long)data[2] << 16) | ((long)data[1] << 8)| (long)data[0]);

  // Construct a 32-bit signed integer
//  value = ( static_cast<unsigned long>(filler) << 24
//  | static_cast<unsigned long>(data[2]) << 16
//  | static_cast<unsigned long>(data[1]) << 8
//  | static_cast<unsigned long>(data[0]) );

//  return static_cast<long>(value);
} 

//Optional spikefilter if AVG_NR is bigger than 3 (smallest and higest nr is discharged). Its nested in AVG_COUNT!, so that rms still can be calculated
long HX::getValue_avg(byte AVG_NR)
{
  long l_min;
  long l_max;
  long current;
  long sum = 0;
  for (int i = 0; i < AVG_NR; i++)
  {
ESP.wdtFeed();
    current = getValue();
    sum += current;
    if (i == 0)
    {
      l_min = current;
      l_max = current;
    }
    else
    {
      l_min = min(l_min, current);
      l_max = max(l_max, current);
    }
  }
  return (sum - l_min - l_max)/(AVG_NR - 2);
}

