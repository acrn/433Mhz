#include <wiringPi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// 5v:         red
// gnd:        brown
// led_pin:    yellow
// radio_pin:  orange

// Signals read:
//
//       "1-on          : 0x65a5a969659a9655\n",
//       "1-off         : 0x65a5a969659a9555\n",
//       "2-on          : 0x65a5a969659a9656\n",
//       "2-off         : 0x65a5a969659a9556\n",
//       "3-on          : 0x65a5a969659a9659\n",
//       "3-off         : 0x65a5a969659a9559\n",
//       "0-all_off     : 0x65a5a969659a9955\n"
//
// It follows that all signals should include:
// on1 & off1 & on2 & off2 & on3 & off3 == 0x65a5a969659a9450
//
#define base_code 0x65a5a969659a9450
// unit codes:
//
//    In [56]: hex(base ^ (on1 & off1))
//    Out[56]: '0x5'
//
#define unit1_code 0x5
//    In [57]: hex(base ^ (on2 & off2))
//    Out[57]: '0x6'
//
#define unit2_code 0x6
//    In [58]: hex(base ^ (on3 & off3))
//    Out[58]: '0x9'
//
#define unit3_code 0x9
// signal codes:
//
//    In [59]: hex(base ^ (on1 & on2 & on3))
//    Out[59]: '0x200'
//
#define on_code 0x200
//    In [60]: hex(base ^ (off1 & off2 & off3))
//    Out[60]: '0x100'
//
#define off_code 0x100


#define led_pin 0
#define radio_pin 7

#define the_long 1322
#define the_short 344
#define the_space 344
#define the_marker 2808

static inline void writePulse(int pin, unsigned int length)
{
    digitalWrite(pin, LOW);
    delayMicroseconds(length);
    digitalWrite(pin, HIGH);
    delayMicroseconds(the_space);
}

static inline void writeUltra(int pin, unsigned int length)
{
    digitalWrite(pin, HIGH);
    delayMicroseconds(length);
    digitalWrite(pin, LOW);
    delayMicroseconds(length);
}

static inline void writeMarker(int pin)
{
    digitalWrite(pin, HIGH);
    delayMicroseconds(the_space);
    writePulse(pin, the_marker);
}

uint64_t signal(uint unit, uint on)
{
    uint unit_code = unit1_code;
    if (unit == 2) unit_code = unit2_code;
    if (unit == 3) unit_code = unit3_code;
    uint on_code_ = off_code;
    if (on) on_code_ = on_code;

    return base_code | unit_code | on_code_;
}

void sendNumber(int pin, uint64_t num, int reps)
{
    for (int i = 63; i >= 0; i--)
    {
        writeMarker(pin);
        for (int j = 63; j >= 0; j--)
        {
            if ((num>>j) & 0x1) {
                writePulse(pin, the_long);
            } else {
                writePulse(pin, the_short);
            }
        }
        digitalWrite(pin, LOW);
    }
}

int main(int argc, char* argv[])
{
  wiringPiSetup () ;
  pinMode (led_pin, OUTPUT) ;
  pinMode (radio_pin, OUTPUT) ;

  /*for (;;)*/
  /*{*/
    /*writePulse(radio_pin, 50000);*/
    /*writePulse(led_pin, 50000);*/
  /*}*/

  uint64_t i2 = 0x8787878787878787LL;

  for (int i = 63; i >= 0; i--)
  {
      if ((i2>>i) & 0x1) {
          printf("long\n");
      } else {
          printf("short\n");
      }
  }

  for (int i = 0; i < argc; i++) {
      char* arg = argv[i];
      uint64_t hej = strtoull(arg, NULL, 16);
      printf("%lx\n", hej);
  }

  /*for (;;)*/
  /*{*/
    /*writeUltra(radio_pin, 500000);*/
    /*writeUltra(led_pin, 500000);*/
  /*}*/

  printf("%lx\n", base_code);

  printf("%lx\n", signal(1,1));
  printf("%lx\n", signal(1,0));
  printf("%lx\n", signal(2,1));
  printf("%lx\n", signal(2,0));
  printf("%lx\n", signal(3,1));
  printf("%lx\n", signal(3,0));

  return 0 ;
}
