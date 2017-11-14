#ifndef HX711_h
#define HX711_h



#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif



class HX
{
  public:
    HX(byte out_pin, byte sck_pin, byte pow_pin);

    HX();

    virtual ~HX();

    int  RMS_TRIES;
    int  rms;   

    void begin(byte out_pin, byte sck_pin, byte pow_pin);
    int get_counts(byte RMS_LIM,byte SNR,byte RMS_TRY, byte AVG_NR,float cal);  

  private:
    bool is_ready();
    void sleep_hx();
    void wake_hx();
    long Read_hx(byte AVG_NR);
    long getValue(); 
    long getValue_avg(byte AVG_NR);
    long RMS();    

 //   float cal;    
    const byte AVG_SIZE = 10;
    long avg;
    long result[10];
    byte SCK_PIN;
    byte OUT_PIN;
    byte POW_PIN;
};

#endif
